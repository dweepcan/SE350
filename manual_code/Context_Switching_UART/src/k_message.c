#include "k_message.h"

int k_send_message(int pid, void *p_msg) {
	// TODO: atomic(on)
	
	p_msg->m_send_pid = gp_current_process->m_pid;
	p_msg->m_recv_pid = pid;
	p_msg->mp_next = NULL;
	ProcessNode* receiving_proc = findProcessNodeByPID(pid);
	msg_enqueue(receiving_proc->pcb->msg_queue, p_msg);
}

void *k_receive_message(int *p_pid) {
}