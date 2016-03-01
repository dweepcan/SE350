/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: Yiqing Huang
 * @date:   2014/02/28
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

int passedTests = 0;
int failedTests = 0;
int testsRun = 0;
char buffer[50];
void *all_memory_blocks[30];

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];//plus last one nullproc

void set_test_procs() {
	int i;
	
	uart0_put_string("G012_test: START\n\r");
	sprintf(buffer, "G012_test: total %d tests\n\r", NUM_TEST_PROCS);
	uart0_put_string((unsigned char*) buffer);
	
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=LOWEST;
		g_test_procs[i].m_stack_size=0x100;
	}
	
 	g_test_procs[0].mpf_start_pc = &proc1;
 	g_test_procs[1].mpf_start_pc = &proc2;
 	g_test_procs[2].mpf_start_pc = &proc3;
 	g_test_procs[3].mpf_start_pc = &proc4;
 	g_test_procs[4].mpf_start_pc = &proc5;
 	g_test_procs[5].mpf_start_pc = &proc6;
}

// Helper function to print out end test results
void printTest(int testNum, int status) {
	sprintf(buffer, "G012_test: test %d %s\n\r", testNum, status==0 ? "FAIL" : "OK");
	uart0_put_string((unsigned char*) buffer);
	testsRun += 1;
	if(status == 0) {
		failedTests += 1;
	} else {
		passedTests += 1;
	}
}

void checkTestEnd() {
	if(testsRun == NUM_TEST_PROCS) {
		sprintf(buffer, "G012_test: %d/%d tests OK\n\r", passedTests, NUM_TEST_PROCS);
		uart0_put_string((unsigned char*) buffer);
		sprintf(buffer, "G012_test: %d/%d tests FAIL\n\r", failedTests, NUM_TEST_PROCS);
		uart0_put_string((unsigned char*) buffer);
		uart0_put_string("G012_test: END\n\r");
	}
}

void proc1(void){
	MSG_BUF *p_msg_env1;
	MSG_BUF *p_msg_env2;
	MSG_BUF *p_msg_env3;
	printf("Entering process 1.\r\n");
	p_msg_env1 = (MSG_BUF *) request_memory_block();
	p_msg_env1->mtype = KCD_REG;
	p_msg_env1->mtext[0] = 'C';
	printf("Send C.\r\n");
	delayed_send(PID_P2,(void *)p_msg_env1, 300);
	p_msg_env2 = (MSG_BUF *) request_memory_block();
	p_msg_env2->mtype = KCD_REG;
	p_msg_env2->mtext[0] = 'A';
	printf("Send B.\r\n");
	delayed_send(PID_P2,(void *)p_msg_env2, 100);
	p_msg_env3 = (MSG_BUF *) request_memory_block();
	p_msg_env3->mtype = KCD_REG;
	p_msg_env3->mtext[0] = 'B';
	printf("Send A.\r\n");
	delayed_send(PID_P2,(void *)p_msg_env3, 200);
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
void proc2(void){
	int dog;
	MSG_BUF *p_msg_rec;
	printf("Entering process 2.\r\n");
	p_msg_rec = (MSG_BUF *)receive_message(&dog);
	printf("Receive A.\r\n");
	printf("Sender ID: %d\n\r", dog);
	printf("Message Type: %d\n\r", p_msg_rec->mtype);
	printf("Message Text: %s\n\r", p_msg_rec->mtext);
	p_msg_rec = (MSG_BUF *)receive_message(&dog);
	printf("Rentering process 2.\r\n");
	printf("Receive B.\r\n");
	printf("Sender ID: %d\n\r", dog);
	printf("Message Type: %d\n\r", p_msg_rec->mtype);
	printf("Message Text: %s\n\r", p_msg_rec->mtext);
	p_msg_rec = (MSG_BUF *)receive_message(&dog);
	printf("Rentering process 2.\r\n");
	printf("Receive C.\r\n");
	printf("Sender ID: %d\n\r", dog);
	printf("Message Type: %d\n\r", p_msg_rec->mtype);
	printf("Message Text: %s\n\r", p_msg_rec->mtext);
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test which ensures we can request a memory block as well as release it after we're done.
void proc3(void){
	printf("Entering process 3.\r\n");
	while(1) {
		release_processor();
	}
}

// Test preemption
void proc4(void){
	printf("Entering process 4.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test which ensures that we correctly set priorities.
void proc5(void){
	printf("Entering process 5.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test whether we can release a memory block which is not actually a memory block, just a random address
void proc6(void){
	printf("Entering process 6.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
