#ifndef K_MSG_H
#define K_MSG_H

#include "k_rtx.h"
#include "k_msg_queue.h"
#include "k_process.h"

extern PCB *gp_current_process;

extern k_msg_queue* pendingMessageQueue;

int k_send_message(int pid, void *p_msg);
extern int k_send_message_nonblocking(int pid, void *p_msg);
void *k_receive_message(int *p_pid);
void *k_receive_message_nonblocking(int *p_pid);
int k_delayed_send(int pid, void *p_msg, int delay);

#endif
