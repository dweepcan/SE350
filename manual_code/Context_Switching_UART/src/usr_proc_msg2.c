// Tests when two processes are sending message to the same process.

// WARNING: p_pid is passed in as NULL in k_receive_messageEntering process 1.
// k_request_memory_block: entering...
// k_request_memory_block: node address: 0x1000165c, block address:0x10001660.
// Entering process 2.
// Sender ID: 1
// Message Type: 0
// Message Text: A
// Entering process 3.
// k_request_memory_block: entering...
// k_request_memory_block: node address: 0x100015d8, block address:0x100015dc.
// Rentering process 2.
// Sender ID: 3
// Message Type: 0
// Message Text: C
// Entering process 4.
// Entering process 5.
// k_request_memory_block: entering...
// k_request_memory_block: node address: 0x10001554, block address:0x10001558.
// Rentering process 4.
// Sender ID: 5
// Message Type: 0
// Message Text: E
// Entering process 6.
// Rentering process 5.

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_memory.h"

//#ifdef DEBUG_0
#include "printf.h"
//#endif /* DEBUG_0 */

int passedTests = 0;
int failedTests = 0;
int testsRun = 0;
char buffer[50];
void *all_memory_blocks[30];

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	
	uart1_put_string("G012_test: START\n\r");
	sprintf(buffer, "G012_test: total %d tests\n\r", NUM_TEST_PROCS);
	uart1_put_string((unsigned char*) buffer);
	
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
	uart1_put_string((unsigned char*) buffer);
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
		uart1_put_string((unsigned char*) buffer);
		sprintf(buffer, "G012_test: %d/%d tests FAIL\n\r", failedTests, NUM_TEST_PROCS);
		uart1_put_string((unsigned char*) buffer);
		uart1_put_string("G012_test: END\n\r");
	}
}

void proc1(void){
	MSG_BUF *p_msg_env;
	printf("Entering process 1.\r\n");
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'A';
	p_msg_env->mtext[1] = '\0';
	send_message(PID_P2,(void *)p_msg_env);
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
void proc2(void){
	int dog;
	MSG_BUF *p_msg_rec = (MSG_BUF *)receive_message(&dog);
	printf("Entering process 2.\r\n");
	printf("Sender ID: %d\n\r", dog);
	printf("Message Type: %d\n\r", p_msg_rec->mtype);
	printf("Message Text: %s\n\r", p_msg_rec->mtext);
	set_process_priority(gp_current_process->m_pid, LOW);
	p_msg_rec = (MSG_BUF *)receive_message(&dog);
	printf("Rentering process 2.\r\n");
	printf("Sender ID: %d\n\r", dog);
	printf("Message Type: %d\n\r", p_msg_rec->mtype);
	printf("Message Text: %s\n\r", p_msg_rec->mtext);
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc3(void){
	MSG_BUF *p_msg_env;
	printf("Entering process 3.\r\n");
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'C';
	p_msg_env->mtext[1] = '\0';
	send_message(PID_P2,(void *)p_msg_env);
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc4(void){
	int dog;
	MSG_BUF *p_msg_rec;
	printf("Entering process 4.\r\n");
	set_process_priority(gp_current_process->m_pid, HIGH);
	p_msg_rec = (MSG_BUF *)receive_message(&dog);
	printf("Rentering process 4.\r\n");
	printf("Sender ID: %d\n\r", dog);
	printf("Message Type: %d\n\r", p_msg_rec->mtype);
	printf("Message Text: %s\n\r", p_msg_rec->mtext);
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc5(void){
	MSG_BUF *p_msg_env;
	printf("Entering process 5.\r\n");
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'E';
	p_msg_env->mtext[1] = '\0';
	send_message(PID_P4,(void *)p_msg_env);
	printf("Rentering process 5.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc6(void){
	printf("Entering process 6.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
