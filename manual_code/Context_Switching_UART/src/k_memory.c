/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/* ----- Global Variables ----- */
// Stack pointer
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */

// Heap pointers
k_stack *gp_heap;
U8 *gp_heap_begin;
U8 *gp_heap_end;

const int NUM_BLOCKS = 30;
const int BLOCK_SIZE = 128; // make this more? AT LEAST 128B?

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
					|---------------------------|
          |   Process Blocked Queue   |
          |---------------------------|
          |   Process Ready Queue     |
          |---------------------------|<--- readyPriorityQueue
          |       ProcessNode 2       |
          |---------------------------|
          |       ProcessNode 1       |
          |---------------------------|
          |   ProcessNodes pointers   |
          |---------------------------|<--- processNodes
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;

	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += (NUM_TEST_PROCS + 1) * sizeof(PCB *);
  
	for ( i = 0; i <= NUM_TEST_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
	printf("gp_pcbs[2] = 0x%x \n", gp_pcbs[2]);
	printf("gp_pcbs[3] = 0x%x \n", gp_pcbs[3]);
	printf("gp_pcbs[4] = 0x%x \n", gp_pcbs[4]);
	printf("gp_pcbs[5] = 0x%x \n", gp_pcbs[5]);
	printf("gp_pcbs[6] = 0x%x \n", gp_pcbs[6]);
#endif
	
	/* allocate memory for pcb pointers   */
	processNodes = (ProcessNode **)p_end;
	p_end += (NUM_TEST_PROCS + 1) * sizeof(ProcessNode *);
	
	for ( i = 0; i <= NUM_TEST_PROCS; i++ ) {
		processNodes[i] = (ProcessNode *)p_end;
		p_end += sizeof(ProcessNode); 
	}
#ifdef DEBUG_0  
	printf("processNodes[0] = 0x%x \n", processNodes[0]);
	printf("processNodes[1] = 0x%x \n", processNodes[1]);
	printf("processNodes[2] = 0x%x \n", processNodes[2]);
	printf("processNodes[3] = 0x%x \n", processNodes[3]);
	printf("processNodes[4] = 0x%x \n", processNodes[4]);
	printf("processNodes[5] = 0x%x \n", processNodes[5]);
	printf("processNodes[6] = 0x%x \n", processNodes[6]);
#endif
	
	for(i = 0; i <= NUM_TEST_PROCS; i++) {
		readyPriorityQueue[i] = (Queue *)p_end;
		readyPriorityQueue[i]->front = NULL;
		readyPriorityQueue[i]->back = NULL;
		p_end += sizeof(Queue);
	}
	
	for(i = 0; i <= NUM_TEST_PROCS; i++) {
		blockedPriorityQueue[i] = (Queue *)p_end;
		blockedPriorityQueue[i]->front = NULL;
		blockedPriorityQueue[i]->back = NULL;
		p_end += sizeof(Queue);
	}
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}

	// OUR CODE
	/* allocate memory for heap, not implemented yet*/
	gp_heap = (k_stack *)p_end;
	gp_heap->top = NULL;
	p_end += sizeof(k_stack);

	/* Save the beginning address of the heap */
	gp_heap_begin = p_end;

	for(i = 0; i < NUM_BLOCKS; i++) {
		/* Create a node representing a memory block */
		k_node *p_node = (k_node *)p_end;

		/* Insert the node into the memory heap stack */
		push(gp_heap, p_node);

		/* Increment the memory address by the size of the memory block */
		p_end += sizeof(k_node) + BLOCK_SIZE;
	}

	/* Save the address of the end of the heap */
	gp_heap_end = p_end;
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void *k_request_memory_block(void) {
	k_node *p_mem_blk = NULL;

#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */

	// TODO: atomic(on) <- need to do this later when time slicing can occur

	while(s_is_empty(gp_heap)) {
		/* If the heap is empty loop until a memory block is available */
#ifdef DEBUG_0
		printf("k_request_memory_block: no available memory blocks.\n");
#endif /* ! DEBUG_0 */
		
		// TODO: Add the process to blocked queue and yield the process
		// current process moved to blocked queue
		// current process state to BLOCKED_ON_RESOURCE
		//if(blockProcess() == RTX_OK) {
		//	release_processor();
		//}
	}

	/* Get the next available node from the heap */
	p_mem_blk = pop(gp_heap);

	/* Increment the address of the node to get the start address of the block */
	p_mem_blk += sizeof(k_node);

	// TODO: atomic(off) <- need to do thi s later when time slicing can occur

#ifdef DEBUG_0
	printf("k_request_memory_block: node address: 0x%x, block address:0x%x.\n", (p_mem_blk - sizeof(k_node)), p_mem_blk);
#endif /* ! DEBUG_0 */

	return (void *) p_mem_blk;
}

int k_release_memory_block(void *p_mem_blk) {
	k_node *p_node = NULL;

#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */

	// TODO: atomic(on) <- need to do this later when time slicing can occur

	if(p_mem_blk == NULL) {
#ifdef DEBUG_0
		printf("k_release_memory_block: cannot release NULL memory block.\n");
#endif /* ! DEBUG_0 */

		return RTX_ERR;
	}

	/* Cast the start address of the memory block to a k_node */
	p_node = (k_node *)p_mem_blk - sizeof(k_node);

	if((U8 *)p_node < gp_heap_begin || (U8 *)p_node > gp_heap_end) {
#ifdef DEBUG_0
		printf("k_release_memory_block: 0x%x is out of bounds of heap memory addresses.\n", p_mem_blk);
#endif /* ! DEBUG_0 */

		return RTX_ERR;
	}

	/* Make sure the memory address is block-aligned */
	if(((U8 *)p_node - gp_heap_begin) % (BLOCK_SIZE + sizeof(k_node)) != 0) {
#ifdef DEBUG_0
		printf("k_release_memory_block: 0x%x is not block-aligned.\n", p_mem_blk);
#endif /* ! DEBUG_0 */

		return RTX_ERR;
	}

	/* Make sure we are not releasing a unallocated memory block */
	if(!s_is_empty(gp_heap) && contains(gp_heap, p_node)) {
#ifdef DEBUG_0
		printf("k_release_memory_block: 0x%x is already in the heap.\n", p_mem_blk);
#endif /* ! DEBUG_0 */

		return RTX_ERR;
	}

	/* Insert the node into the heap */
	if(push(gp_heap, p_node) == RTX_ERR) {
		return RTX_ERR;
	}
	
	// if blocked on resource q not empty
	// handle process ready pop blocked resource q (this should have release processor at some point)
	// assign memory block to the process popped
	//else
	//PCB* nextProcess = getNextBlocked();
	//if(!nextProcess) {
	//	unblockProcess(nextProcess);
	//}

	// TODO: atomic(off) <- need to do this later when time slicing can occur

	return RTX_OK;
}
