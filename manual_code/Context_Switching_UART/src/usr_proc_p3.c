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

#include "printf.h"

/*
Expected output:

G012_test: START
G012_test: total 6 tests
G012_test: test 3 OK
G012_test: test 6 OK
G012_test: test 5 OK
G012_test: test 4 OK
G012_test: test 2 OK
G012_test: test 1 OK
G012_test: 6/6 tests OK
G012_test: 0/6 tests FAIL
G012_test: END
*/


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

// Test changing the priority of a process using the keyboard command with a delayed send (testing 1 char PID as well).
void proc1(void){
	MSG_BUF* p_msg_env;
	int priority = 0;
	int status = 0;
	
	priority = get_process_priority(gp_current_process->m_pid);
	status = priority == LOWEST;
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'C';
	p_msg_env->mtext[2] = ' ';
	p_msg_env->mtext[3] = '1';
	p_msg_env->mtext[4] = ' ';
	p_msg_env->mtext[5] = '0';
	p_msg_env->mtext[6] = '\r';
	p_msg_env->mtext[7] = '\n';
	p_msg_env->mtext[8] = '\0';
	delayed_send(PID_KCD,(void *)p_msg_env, 1000);
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = '1';
	p_msg_env->mtext[1] = '\0';
	delayed_send(PID_P1,(void *)p_msg_env, 1500);
	
	p_msg_env = (MSG_BUF *)receive_message(NULL);
	release_memory_block((void *)p_msg_env);
	
	priority = get_process_priority(PID_KCD);
	status &= priority == HIGH;
	
	printTest(1, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test changing the priority of a process using the keyboard command.
void proc2(void){
	MSG_BUF* p_msg_env;
	int priority = 0;
	int status = 0;
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'C';
	p_msg_env->mtext[2] = ' ';
	p_msg_env->mtext[3] = '1';
	p_msg_env->mtext[4] = '1';
	p_msg_env->mtext[5] = ' ';
	p_msg_env->mtext[6] = '3';
	p_msg_env->mtext[7] = '\r';
	p_msg_env->mtext[8] = '\n';
	p_msg_env->mtext[9] = '\0';
	send_message(PID_KCD,(void *)p_msg_env);
		
	priority = get_process_priority(PID_CLOCK);
	status = priority == LOWEST;
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'C';
	p_msg_env->mtext[2] = ' ';
	p_msg_env->mtext[3] = '1';
	p_msg_env->mtext[4] = '1';
	p_msg_env->mtext[5] = ' ';
	p_msg_env->mtext[6] = '0';
	p_msg_env->mtext[7] = '\r';
	p_msg_env->mtext[8] = '\n';
	p_msg_env->mtext[9] = '\0';
	send_message(PID_KCD,(void *)p_msg_env);
		
	priority = get_process_priority(PID_CLOCK);
	status &= priority == HIGH;
	
	printTest(2, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test which ensures we can request a memory block as well as release it after we're done.
void proc3(void){
	int status;
	void *memory_block = request_memory_block();
	status = release_memory_block(memory_block);
	status = status == RTX_ERR ? 0 : 1;
	printTest(3, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test preemption
void proc4(void){
	set_process_priority(PID_P6, HIGH);
	printTest(4, 1);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test which ensures that we correctly protect against the edge cases of setting priority.
void proc5(void){
	int i = 0;
	int status = 0;
	i += set_process_priority(PID_NULL, HIGH);
	i += set_process_priority(PID_TIMER_IPROC, LOW);
	i += set_process_priority(gp_current_process->m_pid, -1);
	i += set_process_priority(gp_current_process->m_pid, 4);
	status = i==-4;
	printTest(5, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test whether we can release a memory block which is not actually a memory block, just a random address
void proc6(void){
	int status;
	void *memory_block = &status;
	status = release_memory_block(memory_block);
	status = status == RTX_ERR ? 1 : 0;
	printTest(6, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
