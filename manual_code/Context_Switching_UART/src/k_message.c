#include "k_message.h"

int k_send_message(int pid, void *p_msg) {
	__disable_irq();
	
	ProcessNode* sending_proc = findProcessNodeByPID(gp_current_process->m_pid);
	ProcessNode* receiving_proc = findProcessNodeByPID(pid);
	p_msg->m_send_pid = gp_current_process->m_pid;
	p_msg->m_recv_pid = pid;
	p_msg->m_kdata = NULL;
	p_msg->mp_next = NULL;
	msg_enqueue(receiving_proc->pcb->msg_queue, p_msg);
	if(receiving_proc->pcb->state == BLOCKED_ON_RECEIVE) {
		if(unblockReceiveProcess(receiving_proc->pcb) == RTX_ERR) {
			return RTX_ERR;
		}
	}
	
	__enable_irq();
	return RTX_OK;
}

void *k_receive_message(int *p_pid) {
	
	#ifdef DEBUG_0
	if(p_id == NULL) {
			printf("WARNING: p_pid is passed in as NULL in k_receive_message");
	}
	#endif
	
	__disable_irq();
	
	MSG_BUF* msg = NULL;
	ProcessNode* receiving_proc = findProcessNodeByPID(pid);
	while(msg_q_is_empty(receiving_proc->pcb->msg_queue)) {
		blockReceiveProcess();
		__enable_irq();
		k_release_processor();
		__disable_irq();
	}
	msg = msg_dequeue(receiving_proc->pcb->msg_queue);
	if(p_pid != NULL) {
		*p_pid = msg->m_send_pid;
	}
	
	__enable_irq();
	
	return (void *)msg;
}

void *k_receive_message_nonblocking(int *p_pid) {
	// TODO: atomic(on)
	
	#ifdef DEBUG_0
	if(p_id == NULL) {
			printf("WARNING: p_pid is passed in as NULL in k_receive_message");
	}
	#endif
	
	MSG_BUF* msg = NULL;
	ProcessNode* receiving_proc = findProcessNodeByPID(pid);
	if(msg_q_is_empty(receiving_proc->pcb->msg_queue) {
		return NULL;
	} else {
		msg = msg_dequeue(receiving_proc->pcb->msg_queue);
		if(p_pid != NULL) {
			*p_pid = msg->m_send_pid;
		}
	}
	
	// TODO: atomic(off)
	
	return (void *)msg;
}