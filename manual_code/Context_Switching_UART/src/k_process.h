/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef K_PROCESS_H_
#define K_PROCESS_H_

#include "k_rtx.h"
#include "rtx.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */
#define NUM_PRIORITIES 5

struct ProcessNode;
typedef struct ProcessNode{
	PCB* pcb;
	struct ProcessNode* next;
	struct ProcessNode* prev;
} ProcessNode;
 
typedef struct Queue{
	ProcessNode* front;
	ProcessNode* back; 
} Queue;
/* ----- Functions ----- */

void process_init(void);               /* initialize all procs in the system */
PCB *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_processor(void);           /* kernel release_process function */

extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */

extern int isBlockedEmpty(void);
extern ProcessNode* findProcessNodeByPID(int curpid);
extern int addProcessNode(int pid,int priority, int isReady);
extern ProcessNode* removeProcessNode(int process_id,int priority, int isReady);
extern int get_process_priority(int process_id);
extern int isReady(int process_id);
extern int set_process_priority(int process_id, int priority);
extern PCB* getNextBlocked(void);
extern int blockProcess(void);
extern int unblockProcess(PCB* pcb);
extern int process_switch(PCB *p_pcb_old);




#endif /* ! K_PROCESS_H_ */
