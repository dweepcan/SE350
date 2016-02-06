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

// Struct which will Lable a new memory block
typedef struct mem_blk {
  struct mem_blk* next_blk;
} mem_blk;

typedef struct Queue {
	mem_blk *head;
	mem_blk *tail;
} Queue;

// Function which pushes the available memory block onto the queue
void queue_push(Queue *q, mem_blk *block) {
	// TODO: What if the memory block or Queue are null?  Exceptions...
	
	block->next_blk = NULL;
	
	if(q->tail == NULL){
		q->head = block;
	} else {
		q->tail->next_blk = block;
	}
	q->tail = block;
}

// Function which pops the available memory block from the queue if one is available;
mem_blk* queue_pop(Queue *q){
	mem_blk *temp = q->head;
	if(temp == NULL) {
		return NULL;
	}

	if(q->head == q->tail) {
		q->tail = NULL;
	}
	q->head = q->head->next_blk;
	return temp;
}

// Function which checks if a memory block is in the queue of available memory blocks
int queue_contains(Queue *q, void *actual_mem_blk) {
	mem_blk *temp = q->head;
	while(temp) {
		if(actual_mem_blk == (void *)(temp + sizeof(mem_blk))) {
			return RTX_OK;
		}
		temp = temp -> next_blk;
	}
	return RTX_ERR;
}

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */

const int NUM_MEM_BLK = 30;
const int SIZE_MEM_BLK = 128; // make this more? AT LEAST 128B?

// Points to the next available memory block.
mem_blk *head;

Queue heap;

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
	
	// OUR STUFF
	U8 *heap_end;
	mem_blk* current;
	mem_blk block;
  // END OUR STUFF
	
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += NUM_TEST_PROCS * sizeof(PCB *);
  
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
	
	/* allocate memory for heap, not implemented yet*/
	heap_end = p_end;
	head = (mem_blk *)p_end;
	
	for(i = 0; i<NUM_MEM_BLK; i++) {
		current = (mem_blk *)heap_end;
		if(i == NUM_MEM_BLK - 1) {
			heap_end = NULL;
		}
		else {
			heap_end += sizeof(mem_blk) + SIZE_MEM_BLK;
		}
		
		block.next_blk = (mem_blk *)heap_end;
		*current = block;
	}
	
	heap.head = head;
	heap.tail = current;
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
	//mem_blk* test;
	mem_blk *p_mem_blk;
	void *block;
	
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */

	// our code
	// TODO: atomic(on) <- need to do this later when time slicing can occur
	
	while(1) {
		p_mem_blk = queue_pop(&heap);
		if(p_mem_blk) {
			break;
		}
		// current process moved to blocked queue
		// current process state to BLOCKED_ON_RESOURCE
		k_release_processor();
	}
	
	block = p_mem_blk + sizeof(mem_blk);
	
	// TODO: atomic(off) <- need to do this later when time slicing can occur
	
	return block;
	
	// end our code
	
	// return (void *) NULL;
}

int k_release_memory_block(void *p_mem_blk) {
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
	
	// our code
	// TODO: atomic(on) <- need to do this later when time slicing can occur
	
	if(queue_contains(&heap, p_mem_blk) == RTX_ERR) {
		return RTX_ERR;
	}
	
	// if blocked on resource q not empty
	// handle process ready pop blocked resource q (this should have release processor at some point)
	// assign memory block to the process popped
	//else
	queue_push(&heap, (mem_blk *)p_mem_blk - sizeof(mem_blk));
	
	// TODO: atomic(off) <- need to do this later when time slicing can occur
	// end our code
	
	return RTX_OK;
}
