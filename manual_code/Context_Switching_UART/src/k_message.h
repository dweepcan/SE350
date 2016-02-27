#ifndef K_MSG_H
#define K_MSG_H

#include "k_rtx.h"
#include "k_msg_queue.h"

extern PCB *gp_current_process;
extern Queue *readyPriorityQueue[NUM_PRIORITIES];
extern Queue *blockedPriorityQueue[NUM_PRIORITIES];
extern ProcessNode **processNodes;

int k_send_message(int pid, void *p_msg);
void *k_receive_message(int *p_pid);

#endif