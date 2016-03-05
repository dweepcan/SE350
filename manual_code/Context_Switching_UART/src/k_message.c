#include "k_message.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

extern volatile U32 g_timer_count;

int k_send_message(int pid, void *p_msg) {
	ProcessNode* receiving_proc;
	MSG_BUF* p_msg_buf;
	
	__disable_irq();
	
	if(pid < 0 || pid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1)) {
		__enable_irq();
		return RTX_ERR;
	}

	receiving_proc = findProcessNodeByPID(pid);
	p_msg_buf = (MSG_BUF *)p_msg;
	p_msg_buf->m_send_pid = gp_current_process->m_pid;
	p_msg_buf->m_recv_pid = pid;
	p_msg_buf->mp_next = NULL;
	msg_enqueue(receiving_proc->pcb->msg_queue, p_msg_buf);
	if(receiving_proc->pcb->m_state == BLOCKED_ON_RECEIVE) {
		if(unblockReceiveProcess(receiving_proc->pcb) == RTX_ERR) {
			__enable_irq();
			return RTX_ERR;
		}
		
		//preempt :)
		//highest priority is 0
		if (receiving_proc->pcb->m_priority < gp_current_process->m_priority){
			__enable_irq();
			k_release_processor();
			__disable_irq();
		}
	}
	
	__enable_irq();
	return RTX_OK;
}

int k_send_message_nonblocking(int pid, void *p_msg) {
	ProcessNode* receiving_proc;
	MSG_BUF* p_msg_buf;
	
	if(pid < 0 || pid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1)) {
		return RTX_ERR;
	}

	receiving_proc = findProcessNodeByPID(pid);
	p_msg_buf = (MSG_BUF *)p_msg;
	msg_enqueue(receiving_proc->pcb->msg_queue, p_msg_buf);
	if(receiving_proc->pcb->m_state == BLOCKED_ON_RECEIVE) {
		if(unblockReceiveProcess(receiving_proc->pcb) == RTX_ERR) {
			return RTX_ERR;
		}
	}
	
	return RTX_OK;
}

void *k_receive_message(int *p_pid) {
	MSG_BUF* msg;
	ProcessNode* receiving_proc;
	
	__disable_irq();
	
	#ifdef DEBUG_0
	if(p_pid == NULL) {
			printf("WARNING: p_pid is passed in as NULL in k_receive_message\n\r");
	}
	#endif
		
	msg = NULL;
	receiving_proc = findProcessNodeByPID(gp_current_process->m_pid);
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
	MSG_BUF* msg;
	ProcessNode* receiving_proc;
	
	#ifdef DEBUG_0
	if(p_pid == NULL) {
			printf("WARNING: p_pid is passed in as NULL in k_receive_message");
	}
	#endif
	
	msg = NULL;
	receiving_proc = findProcessNodeByPID(gp_current_process->m_pid);
	if(msg_q_is_empty(receiving_proc->pcb->msg_queue)) {
		return NULL;
	} else {
		msg = msg_dequeue(receiving_proc->pcb->msg_queue);
		if(p_pid != NULL) {
			*p_pid = msg->m_send_pid;
		}
	}
	
	return (void *)msg;
}

int k_delayed_send(int pid, void *p_msg, int delay) {
	int returnState = RTX_OK;
	MSG_BUF* msg;

	__disable_irq();
	
	if(pid < 1 || pid > (NUM_TEST_PROCS + NUM_SYS_PROCS - 1) || delay < 0 || p_msg == NULL) {
		__enable_irq();
		return RTX_ERR;
	}
	
	// TODO: If doing a delayed send to an i-process then this might fail.
	if(delay == 0) {
		__enable_irq();
		return send_message(pid, p_msg);
	}
	
	msg = (MSG_BUF*) p_msg;
	msg->m_send_pid = gp_current_process->m_pid;
	msg->m_recv_pid = pid;
	msg->m_kdata[0] = delay + (int)g_timer_count;
	msg->mp_next = NULL;
	if(msg_enqueue(pendingMessageQueue, msg) == RTX_ERR) {
		returnState = RTX_ERR;
	}
	
	__enable_irq();
	
	return returnState;
}
