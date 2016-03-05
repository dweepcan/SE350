/** 
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_RTX_H_
#define K_RTX_H_

/*----- Definitions -----*/
#include "k_msg_queue.h"

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 6
#define NUM_SYS_PROCS 5
#define NUM_USER_PROCS 5

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 218B  */
#endif /* DEBUG_0 */

/*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/* process states, note we only assume three states in this example */
typedef enum {NEW = 0, RDY, RUN, BLOCKED_ON_RESOURCE, BLOCKED_ON_RECEIVE} PROC_STATE_E;  

/* initialization table item */
#ifndef PROC_INIT_
#define PROC_INIT_

typedef struct proc_init
{	
	int m_pid;	        /* process id */ 
	int m_priority;         /* initial priority, not used in this example. */ 
	int m_stack_size;       /* size of stack in words */
	void (*mpf_start_pc) ();/* entry point of the process */    
} PROC_INIT;

/* message buffer */
typedef struct msgbuf
{
#ifdef K_MSG_ENV
	void *mp_next;		/* ptr to next message received*/
	int m_send_pid;		/* sender pid */
	int m_recv_pid;		/* receiver pid */
	int m_kdata[5];		/* extra 20B kernel data place holder */
	
#endif
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
} MSG_BUF;

#endif

/*
  PCB data structure definition.
  You may want to add your own member variables
  in order to finish P1 and the entire project 
*/
typedef struct pcb 
{ 
	//struct pcb *mp_next;  /* next pcb, not used in this example */  
	U32 *mp_sp;		/* stack pointer of the process */
	U32 m_pid;		/* process id */
	U32 m_priority;		/* process priority */
	PROC_STATE_E m_state;   /* state of the process */
	k_msg_queue* msg_queue;		/* Adds message queue for PCB */
} PCB;

#endif // ! K_RTX_H_
