#ifndef K_MSG_H
#define K_MSG_H

#include "k_rtx.h"
#include "k_msg_queue.h"
#include "k_process.h"

extern PCB *gp_current_process;

int k_send_message(int pid, void *p_msg);
void *k_receive_message(int *p_pid);
void *k_receive_message_nonblocking(int *p_pid);

#endif
