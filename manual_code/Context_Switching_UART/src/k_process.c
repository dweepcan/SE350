/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/02/28
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */

U32 g_switch_flag = 0;          /* whether to continue to run the process before the UART receive interrupt */
                                /* 1 means to switch to another process, 0 means to continue the current process */
				/* this value will be set by UART handler */

/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS + NUM_SYS_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];
extern PROC_INIT g_sys_procs[NUM_SYS_PROCS];

/**
 * @brief: initialize all processes in the system
 * NOTE: We assume there are only six user processes in the system in this example.
 */
Queue *readyPriorityQueue[NUM_PRIORITIES];
Queue *blockedPriorityQueue[NUM_PRIORITIES];
ProcessNode **processNodes;


int isBlockedEmpty(){
	int i;
	for (i=0; i<NUM_PRIORITIES; i++){
		if (blockedPriorityQueue[i]->front != NULL) return 1;		
	}
	return 0;
}


ProcessNode* findProcessNodeByPID(int curpid){
	if (curpid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1) || curpid < 0) return NULL;
	return processNodes[curpid];
}


int addProcessNode(int pid,int priority, int isReady){
	ProcessNode* pn;
	if (pid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1)) return RTX_ERR;
	pn = findProcessNodeByPID(pid);
	
	if (isReady==0) { //if ready
		if (readyPriorityQueue[priority]->front == NULL){
			readyPriorityQueue[priority]->back = pn;
			readyPriorityQueue[priority]->front = pn;
			pn->next = NULL;
			pn->prev = NULL;
			return RTX_OK;
		}
	
		readyPriorityQueue[priority]->back->next = pn;
		pn->prev = readyPriorityQueue[priority]->back;
		pn->next = NULL;
		readyPriorityQueue[priority]->back = pn;
		return RTX_OK;
	} else if (isReady==1) { //if blocked
		if (blockedPriorityQueue[priority]->front == NULL){
			blockedPriorityQueue[priority]->back = pn;
			blockedPriorityQueue[priority]->front = pn;
			pn->next = NULL;
			pn->prev = NULL;
			return RTX_OK;
		}
	
		blockedPriorityQueue[priority]->back->next = pn;
		pn->prev = blockedPriorityQueue[priority]->back;
		pn->next = NULL;
		blockedPriorityQueue[priority]->back = pn;
		return RTX_OK;
	}
	
	return RTX_ERR;
}

ProcessNode* removeProcessNode(int process_id,int priority, int isReady){
	ProcessNode* returnNode= NULL;
	
	if (isReady == 0){ //if ready
		if ((readyPriorityQueue[priority])->back->pcb->m_pid == process_id && (readyPriorityQueue[priority])->front->pcb->m_pid == process_id){
			returnNode = readyPriorityQueue[priority]->back;
			readyPriorityQueue[priority]->back=NULL;
			readyPriorityQueue[priority]->front=NULL;
			return returnNode;
		}
		if (readyPriorityQueue[priority]->back->pcb->m_pid == process_id){
			returnNode = readyPriorityQueue[priority]->back;	
			readyPriorityQueue[priority]->back = readyPriorityQueue[priority]->back->prev;
			readyPriorityQueue[priority]->back->next = NULL;
			return returnNode;
		}
		if (readyPriorityQueue[priority]->front->pcb->m_pid == process_id){
			returnNode = readyPriorityQueue[priority]->front;
			readyPriorityQueue[priority]->front = readyPriorityQueue[priority]->front->next;
			readyPriorityQueue[priority]->front->prev = NULL;
			return returnNode;
		}
		
		returnNode = readyPriorityQueue[priority]->front->next;
		while (returnNode != readyPriorityQueue[priority]->back){
			if (returnNode->pcb->m_pid == process_id){
				returnNode->prev->next =  returnNode->next;
				returnNode->next->prev =  returnNode->prev;
				return returnNode;
			}
			returnNode=returnNode->next;
		}
		
		return NULL;

	}else if(isReady == 1){ //if blocked
		if (blockedPriorityQueue[priority]->back->pcb->m_pid == process_id && blockedPriorityQueue[priority]->front->pcb->m_pid == process_id){
			returnNode = blockedPriorityQueue[priority]->back;
			blockedPriorityQueue[priority]->back=NULL;
			blockedPriorityQueue[priority]->front=NULL;
			return returnNode;
		}
		if (blockedPriorityQueue[priority]->back->pcb->m_pid == process_id){
			returnNode = blockedPriorityQueue[priority]->back;	
			blockedPriorityQueue[priority]->back = blockedPriorityQueue[priority]->back->prev;
			blockedPriorityQueue[priority]->back->next = NULL;
			return returnNode;
		}
		if (blockedPriorityQueue[priority]->front->pcb->m_pid == process_id){
			returnNode = blockedPriorityQueue[priority]->front;
			blockedPriorityQueue[priority]->front = blockedPriorityQueue[priority]->front->next;
			blockedPriorityQueue[priority]->front->prev = NULL;
			return returnNode;
		}
		
		returnNode = blockedPriorityQueue[priority]->front->next;
		while (returnNode != blockedPriorityQueue[priority]->back){
			if (returnNode->pcb->m_pid == process_id){
				returnNode->prev->next =  returnNode->next;
				returnNode->next->prev =  returnNode->prev;
				return returnNode;
			}
			returnNode=returnNode->next;
		}
		
		return NULL;
	}
	return NULL;
}

//get the process priority :)
int k_get_process_priority(int process_id){
	ProcessNode* node = findProcessNodeByPID(process_id);
	if (node == NULL) return RTX_ERR;
	return node->pcb->m_priority;
}

//checks if a process is in the ready state (0) or blocked (1), -1 if not found 
int isReady(int process_id){
	int i;
	for (i=0; i<NUM_PRIORITIES; i++){
		ProcessNode* tempNode = readyPriorityQueue[i]->front;
		while(tempNode!=NULL){
			if (tempNode->pcb->m_pid == process_id) return 0;
			tempNode = tempNode->next;
		}
		tempNode = blockedPriorityQueue[i]->front;
		while(tempNode!=NULL){
			if (tempNode->pcb->m_pid == process_id) return 1;
			tempNode = tempNode->next;
		}
	}
	
	return -1;
}

int k_set_process_priority(int process_id, int priority){
	int state = isReady(process_id);
	int oldPriority = 0;
	//todo update isready
	ProcessNode* oldNode = findProcessNodeByPID(process_id);
	
	//prevent set process if modifying null proc or setting priority to null proc level
	if (process_id == 0 || priority == 4 || oldNode == NULL){
		return RTX_ERR;
	}
	
	if (process_id != gp_current_process->m_pid){
		removeProcessNode(process_id,oldNode->pcb->m_priority,state);
		addProcessNode(process_id,priority,state);
	}
	
	oldPriority = oldNode->pcb->m_priority;
	oldNode->pcb->m_priority = priority;
	//preempt :)
	//highest priority is 0
	if ((gp_current_process->m_pid != process_id && priority <= gp_current_process->m_priority) ||
		(gp_current_process->m_pid == process_id && priority > oldPriority)){
		k_release_processor();
	}
	
	return RTX_OK;
}


void process_init() {
	int i;
	U32 *sp;
  
	/* fill out the initialization table */
	set_test_procs();
	set_sys_procs();
	
	//set queues to null
	for (i=0; i<5; i++) {
		(readyPriorityQueue[i])->front = NULL;
		(readyPriorityQueue[i])->back = NULL;

		blockedPriorityQueue[i]->front = NULL;
		blockedPriorityQueue[i]->back = NULL;
	}
	
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
	}
	
	for ( i = 0; i < NUM_SYS_PROCS; i++ ) {
		g_proc_table[NUM_TEST_PROCS + i].m_pid = g_sys_procs[i].m_pid;
		g_proc_table[NUM_TEST_PROCS + i].m_stack_size = g_sys_procs[i].m_stack_size;
		g_proc_table[NUM_TEST_PROCS + i].mpf_start_pc = g_sys_procs[i].mpf_start_pc;
		g_proc_table[NUM_TEST_PROCS + i].m_priority = g_sys_procs[i].m_priority;
	}
	
	(processNodes[0])->pcb = gp_pcbs[NUM_TEST_PROCS];
	(processNodes[0])->next = NULL;
	(processNodes[0])->prev = NULL;
	
	for(i=1; i <= NUM_TEST_PROCS; i++){
		(processNodes[i])->pcb = gp_pcbs[i-1];
		(processNodes[i])->next = NULL;
		(processNodes[i])->prev = NULL;
	}
  
	/* initialize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < (NUM_TEST_PROCS + NUM_SYS_PROCS); i++ ) {
		int j;

		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
		addProcessNode((gp_pcbs[i])->m_pid, gp_pcbs[i]->m_priority,0);
	}
}



PCB* getNextBlocked(void){
	int i=0; 
	for(i=0; i<4; i++){
		if(blockedPriorityQueue[i]->front != NULL){			
			return blockedPriorityQueue[i]->front->pcb;
		}
	}
	return NULL;
}

int blockProcess(void){
	
	if(gp_current_process != NULL){
		ProcessNode* node = findProcessNodeByPID(gp_current_process->m_pid);
		if (node==NULL) return RTX_ERR;
	
		node->pcb->m_state = BLOCKED;
		addProcessNode(gp_current_process->m_pid, gp_current_process->m_priority,1); //add to blocked(1)
		
		return RTX_OK;
	}
	
	return RTX_ERR;
}


int unblockProcess(PCB* pcb){

	if (pcb != NULL){
		ProcessNode * node = removeProcessNode(pcb->m_pid, pcb->m_priority, 1);	
		pcb->m_state = RDY;

		addProcessNode(pcb->m_pid, pcb->m_priority, 0);
	
		//preempt :(
		//highest priority is 0
		if (pcb->m_priority <= gp_current_process->m_priority){
			k_release_processor();
		}
			return RTX_OK;
	}
	return RTX_ERR;
}
/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

PCB *scheduler(void){
	int i;
	if (gp_current_process != NULL && gp_current_process->m_state != BLOCKED) {
		//should only be false at first
		addProcessNode(gp_current_process->m_pid,gp_current_process->m_priority,0);//put it at the back of the same pri ready q
	}
	for (i=0;i<=4;i++){
			if (readyPriorityQueue[i]->front !=NULL){
					gp_current_process=readyPriorityQueue[i]->front->pcb;
					removeProcessNode(gp_current_process->m_pid,i,0);
					//gp_current_process->m_state = RUN;
					return gp_current_process;
			}
	}
	
	return gp_current_process;
}



/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old) {
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != BLOCKED) {
			p_pcb_old->m_state = RDY;
		}
		
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW){
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){
			if(p_pcb_old->m_state != BLOCKED) {
				p_pcb_old->m_state = RDY; 
			}
			
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}
/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void){
	PCB *p_pcb_old = NULL;
	
	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();
	
	
	//Error checking code: should not happen 
	if ( gp_current_process == NULL  ) {
		gp_current_process = p_pcb_old; // revert back tog the old process
		return RTX_ERR;
	}
        if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	}
	process_switch(p_pcb_old);
	return RTX_OK;
}
