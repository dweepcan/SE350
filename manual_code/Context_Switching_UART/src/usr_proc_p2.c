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

//#ifdef DEBUG_0
#include "printf.h"
//#endif /* DEBUG_0 */

int passedTests = 0;
int failedTests = 0;
int testsRun = 0;
char buffer[50];
void *all_memory_blocks[30];

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];//plus last one nullproc

// Initialize test procs
void set_test_procs() {
	int i;
	
	uart1_put_string("G012_test: START\n\r");
	sprintf(buffer, "G012_test: total %d tests\n\r", NUM_TEST_PROCS);
	uart1_put_string((unsigned char*) buffer);
	
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=HIGH;
		g_test_procs[i].m_stack_size=0x100;
	}
	
	g_test_procs[4].m_priority=MEDIUM;
	g_test_procs[5].m_priority=MEDIUM;
	
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

// Helper function which will print the test results once the tests are done
void checkTestEnd() {
	if(testsRun == NUM_TEST_PROCS) {
		sprintf(buffer, "G012_test: %d/%d tests OK\n\r", passedTests, NUM_TEST_PROCS);
		uart1_put_string((unsigned char*) buffer);
		sprintf(buffer, "G012_test: %d/%d tests FAIL\n\r", failedTests, NUM_TEST_PROCS);
		uart1_put_string((unsigned char*) buffer);
		uart1_put_string("G012_test: END\n\r");
	}
}

// Process 1: Quits upon receiving the keyboard shortcut registered
void proc1(void){
	int sender_id;
	MSG_BUF *p_msg_env;
	
#ifdef DEBUG_0
	printf("Entering process 1\r\n");
#endif	
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = KCD_REG;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'A';
	p_msg_env->mtext[2] = '\0';
	send_message(PID_KCD,(void *)p_msg_env);
#ifdef DEBUG_0
	printf("Process 1 sends %%A KCD_REG\r\n");
#endif
	
	p_msg_env = (MSG_BUF *)receive_message(&sender_id);
#ifdef DEBUG_0
	printf("PROC 2 Test result: %s from process %d\r\n", p_msg_env->mtext, sender_id);
#endif
	release_memory_block(p_msg_env);
	
	printTest(1, 1);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Process 2: Registers a command, will exit when it gets the right arguments
void proc2(void){
	int sender_id;
	MSG_BUF *p_msg_env;
	
#ifdef DEBUG_0
	printf("Entering process 2\r\n");
#endif
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = KCD_REG;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'B';
	p_msg_env->mtext[2] = '\0';
	send_message(PID_KCD,(void *)p_msg_env);
#ifdef DEBUG_0
	printf("Process 2 sends %%B KCD_REG\r\n");
#endif
	while(1) {
		p_msg_env = (MSG_BUF *)receive_message(&sender_id);
		if(p_msg_env -> mtext[3] == 'Y' && p_msg_env -> mtext[4] == '\0') {
			break;
		}
	}
	
	printTest(2, 1);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Process 3: Start up a wall clock, and then stop it later myself
void proc3(void){
	int sender_id;
	MSG_BUF *p_msg_env;

#ifdef DEBUG_0
	printf("Entering process 3\r\n");
#endif	

	// Start up the wall clock at a time which will wrap around
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'W';
	p_msg_env->mtext[2] = 'S';
	p_msg_env->mtext[3] = ' ';
	p_msg_env->mtext[4] = '2';
	p_msg_env->mtext[5] = '3';
	p_msg_env->mtext[6] = ':';
	p_msg_env->mtext[7] = '5';
	p_msg_env->mtext[8] = '9';
	p_msg_env->mtext[9] = ':';
	p_msg_env->mtext[10] = '5';
	p_msg_env->mtext[11] = '4';
	p_msg_env->mtext[12] = '\r';
	p_msg_env->mtext[13] = '\n';
	p_msg_env->mtext[14] = '\0';
	send_message(PID_KCD,(void *)p_msg_env);
	
	// Send a delayed message to myself to stop the wall clock in a minute
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtext[0] = '1';
	p_msg_env->mtext[1] = '\0';
	delayed_send(PID_P3,(void *)p_msg_env, 10000);
	
	// Wait until I receive the message from myself
	while(1) {
		p_msg_env = (MSG_BUF *)receive_message(&sender_id);
		release_memory_block((void *)p_msg_env);
		
		// Kill the wall clock
		if(sender_id == PID_P3) {
			p_msg_env = (MSG_BUF *) request_memory_block();
			p_msg_env->mtype = DEFAULT;
			p_msg_env->mtext[0] = '%';
			p_msg_env->mtext[1] = 'W';
			p_msg_env->mtext[2] = 'T';
			p_msg_env->mtext[3] = '\r';
			p_msg_env->mtext[4] = '\n';
			p_msg_env->mtext[5] = '\0';
			send_message(PID_KCD,(void *)p_msg_env);
			break;
		}
	}
	
	printTest(3, 1);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Process 4: Always ready to bar entry into procs 5 and 6
void proc4(void){
#ifdef DEBUG_0
	printf("Entering process 4\r\n");
#endif
	
	while(1) {
		if(testsRun == 3) {
			break;
		}
		release_processor();
	}
	
	printTest(4, 1);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Process 5: Takes up all the memory remaining for 10 seconds, so that we can have blocked on receive and blocked on memory at the same time
void proc5(void){
	int sender_id, i, status;
	MSG_BUF *p_msg_env;
	
#ifdef DEBUG_0
	printf("Entering process 5\r\n");
#endif
	
	set_process_priority(gp_current_process->m_pid, HIGH);
	
	for(i = 0; i < 28; i++) {
		all_memory_blocks[i] = request_memory_block();
	}
	
	p_msg_env = (MSG_BUF *) request_memory_block();
	p_msg_env->mtext[0] = '1';
	p_msg_env->mtext[1] = '\0';
	delayed_send(PID_P5,(void *)p_msg_env, 10000);
	
	while(1) {
		p_msg_env = (MSG_BUF *)receive_message(&sender_id);
		release_memory_block((void *)p_msg_env);
		if(sender_id == PID_P5) {
			break;
		}
	}
	
	for(i = 0; i < 29; i++) {
		status &= (release_memory_block(all_memory_blocks[i]) == RTX_OK) ? 1 : 0;
	}
	
	printTest(5, 1);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Process 6: Get blocked on memory, once unblocked immediately release
void proc6(void){
	int status;
	void *testBlock;
	
#ifdef DEBUG_0
	printf("Entering process 6\r\n");
#endif
	
	testBlock = request_memory_block();
	status = release_memory_block(testBlock);
	
	status = status == RTX_OK ? 1 : 0;
	printTest(6, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
