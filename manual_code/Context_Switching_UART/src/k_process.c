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
k_msg_queue **gp_msgs;					/* array of k_msg_queue */
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
Queue *blockedResourceQueue[NUM_PRIORITIES];
Queue *blockedReceiveQueue;
ProcessNode **processNodes;


int isBlockedEmpty(){
	//checks if blocked on resource is empty
	int i;
	for (i=0; i<NUM_PRIORITIES; i++){
		if (blockedResourceQueue[i]->front != NULL) return 0;		
	}
	return 1;
}


ProcessNode* findProcessNodeByPID(int curpid){
	if (curpid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1) || curpid < 0) return NULL;
	return processNodes[curpid];
}


int addProcessNode(int pid,int priority, int state){
	ProcessNode* pn;
	if (pid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1)) return RTX_ERR;
	pn = findProcessNodeByPID(pid);
	
	if (state==RDY || state==NEW) { //if ready
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
	} else if (state==BLOCKED_ON_RESOURCE) { //if blocked_resource
		if (blockedResourceQueue[priority]->front == NULL){
			blockedResourceQueue[priority]->back = pn;
			blockedResourceQueue[priority]->front = pn;
			pn->next = NULL;
			pn->prev = NULL;
			return RTX_OK;
		}
	
		blockedResourceQueue[priority]->back->next = pn;
		pn->prev = blockedResourceQueue[priority]->back;
		pn->next = NULL;
		blockedResourceQueue[priority]->back = pn;
		return RTX_OK;
	}else if (state==BLOCKED_ON_RECEIVE){
		if (blockedReceiveQueue->front == NULL){
			blockedReceiveQueue->back = pn;
			blockedReceiveQueue->front = pn;
			pn->next = NULL;
			pn->prev = NULL;
			return RTX_OK;
		}
		blockedReceiveQueue->back->next = pn;
		pn->prev = blockedReceiveQueue->back;
		pn->next = NULL;
		blockedReceiveQueue->back = pn;
		return RTX_OK;
		
	}
	
	return RTX_ERR;
}

ProcessNode* removeProcessNode(int process_id,int priority, int state){
	ProcessNode* returnNode= NULL;
	
	if (state == RDY || state == NEW){ //if ready
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

	}else if(state == BLOCKED_ON_RESOURCE){ //if blocked
		if (blockedResourceQueue[priority]->back->pcb->m_pid == process_id && blockedResourceQueue[priority]->front->pcb->m_pid == process_id){
			returnNode = blockedResourceQueue[priority]->back;
			blockedResourceQueue[priority]->back=NULL;
			blockedResourceQueue[priority]->front=NULL;
			return returnNode;
		}
		if (blockedResourceQueue[priority]->back->pcb->m_pid == process_id){
			returnNode = blockedResourceQueue[priority]->back;	
			blockedResourceQueue[priority]->back = blockedResourceQueue[priority]->back->prev;
			blockedResourceQueue[priority]->back->next = NULL;
			return returnNode;
		}
		if (blockedResourceQueue[priority]->front->pcb->m_pid == process_id){
			returnNode = blockedResourceQueue[priority]->front;
			blockedResourceQueue[priority]->front = blockedResourceQueue[priority]->front->next;
			blockedResourceQueue[priority]->front->prev = NULL;
			return returnNode;
		}
		
		returnNode = blockedResourceQueue[priority]->front->next;
		while (returnNode != blockedResourceQueue[priority]->back){
			if (returnNode->pcb->m_pid == process_id){
				returnNode->prev->next =  returnNode->next;
				returnNode->next->prev =  returnNode->prev;
				return returnNode;
			}
			returnNode=returnNode->next;
		}
		
		return NULL;
	}else  if(state == BLOCKED_ON_RECEIVE){ //if blocked
		if (blockedReceiveQueue->back->pcb->m_pid == process_id && blockedReceiveQueue->front->pcb->m_pid == process_id){
			returnNode = blockedReceiveQueue->back;
			blockedReceiveQueue->back=NULL;
			blockedReceiveQueue->front=NULL;
			return returnNode;
		}
		if (blockedReceiveQueue->back->pcb->m_pid == process_id){
			returnNode = blockedReceiveQueue->back;	
			blockedReceiveQueue->back = blockedReceiveQueue->back->prev;
			blockedReceiveQueue->back->next = NULL;
			return returnNode;
		}
		if (blockedReceiveQueue->front->pcb->m_pid == process_id){
			returnNode = blockedReceiveQueue->front;
			blockedReceiveQueue->front = blockedReceiveQueue->front->next;
			blockedReceiveQueue->front->prev = NULL;
			return returnNode;
		}
		
		returnNode = blockedReceiveQueue->front->next;
		while (returnNode != blockedReceiveQueue->back){
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
int getState(int process_id){
	int i;
	for (i=0; i<NUM_PRIORITIES; i++){
		ProcessNode* tempNode = readyPriorityQueue[i]->front;
		while(tempNode!=NULL){
			if (tempNode->pcb->m_pid == process_id) return RDY;
			tempNode = tempNode->next;
		}
		tempNode = blockedResourceQueue[i]->front;
		while(tempNode!=NULL){
			if (tempNode->pcb->m_pid == process_id) return BLOCKED_ON_RESOURCE;
			tempNode = tempNode->next;
		}
		
		tempNode = blockedReceiveQueue->front;
		while (tempNode!=NULL){
			if (tempNode->pcb->m_pid == process_id) return BLOCKED_ON_RECEIVE;
			tempNode = tempNode->next;
		}
	//todo add antoher while loop
	}
	
	return -1;
}

int k_set_process_priority(int process_id, int priority){
	int state = getState(process_id);
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
	//TODO: Call release processor only when changing the priority of a non-blocked process 
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

		blockedResourceQueue[i]->front = NULL;
		blockedResourceQueue[i]->back = NULL;
	}
	blockedReceiveQueue->front = NULL;
	blockedReceiveQueue->back = NULL;
	
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
  
	pending_message_queue_init(); // Set up the pending message queue
	
	/* initialize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < (NUM_TEST_PROCS + NUM_SYS_PROCS); i++ ) {
		int j;

		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->msg_queue = gp_msgs[i];
		(gp_pcbs[i])->msg_queue->first = NULL;
		(gp_pcbs[i])->msg_queue->last = NULL;
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
		addProcessNode((gp_pcbs[i])->m_pid, gp_pcbs[i]->m_priority,RDY);
	}
}



PCB* getNextBlocked(void){
	int i=0; 
	for(i=0; i<4; i++){
		if(blockedResourceQueue[i]->front != NULL){			
			return blockedResourceQueue[i]->front->pcb;
		}
	}
	return NULL;
}

int blockProcess(void){
	
	if(gp_current_process != NULL){
		ProcessNode* node = findProcessNodeByPID(gp_current_process->m_pid);
		if (node==NULL) return RTX_ERR;
	
		node->pcb->m_state = BLOCKED_ON_RESOURCE;
		addProcessNode(gp_current_process->m_pid, gp_current_process->m_priority,BLOCKED_ON_RESOURCE); //add to blocked(1)
		
		return RTX_OK;
	}
	
	return RTX_ERR;
}


int unblockProcess(PCB* pcb){
	
	if (pcb != NULL){
		ProcessNode * node = removeProcessNode(pcb->m_pid, pcb->m_priority, BLOCKED_ON_RESOURCE);	
		pcb->m_state = RDY;

		addProcessNode(pcb->m_pid, pcb->m_priority, RDY);
	
		//preempt :(
		//highest priority is 0
		if (pcb->m_priority <= gp_current_process->m_priority){
			k_release_processor();
		}
			return RTX_OK;
	}
	return RTX_ERR;
}


int blockReceiveProcess(){
		if(gp_current_process != NULL){
		ProcessNode* node = findProcessNodeByPID(gp_current_process->m_pid);
		if (node==NULL) return RTX_ERR;
	
		node->pcb->m_state = BLOCKED_ON_RECEIVE;
		addProcessNode(gp_current_process->m_pid, gp_current_process->m_priority,BLOCKED_ON_RECEIVE); //add to blocked(Receive
		return RTX_OK;
	}
	
	return RTX_ERR;
}
int unblockReceiveProcess(PCB* pcb){
	if (pcb != NULL){
		ProcessNode * node = removeProcessNode(pcb->m_pid, pcb->m_priority, BLOCKED_ON_RECEIVE);	
		pcb->m_state = RDY;

		addProcessNode(pcb->m_pid, pcb->m_priority, RDY);
	
		//preempt :)
		//highest priority is 0
		if (pcb->m_priority < gp_current_process->m_priority){
			__enable_irq();
			k_release_processor();
			__disable_irq();
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
	if (gp_current_process != NULL && gp_current_process->m_state != BLOCKED_ON_RESOURCE && gp_current_process->m_state != BLOCKED_ON_RECEIVE) {
		//should only be false at first
		addProcessNode(gp_current_process->m_pid,gp_current_process->m_priority,RDY);//put it at the back of the same pri ready q
	}
	for (i=0;i<=4;i++){
			if (readyPriorityQueue[i]->front !=NULL){
					gp_current_process=readyPriorityQueue[i]->front->pcb;
					removeProcessNode(gp_current_process->m_pid,i,RDY);
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
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != BLOCKED_ON_RESOURCE && p_pcb_old->m_state != BLOCKED_ON_RECEIVE) {
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
			if(p_pcb_old->m_state != BLOCKED_ON_RESOURCE && p_pcb_old->m_state != BLOCKED_ON_RECEIVE) {
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


void printQueue(PROC_STATE_E state){
	if (state == RDY){
		printf("ready");
	}else if (state == BLOCKED_ON_RESOURCE){
		printf("memory");
	}else if (state == BLOCKED_ON_RECEIVE){
		printf("receive");
	}
}
