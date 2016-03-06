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

// Take up all the memory to test blocking on resource
void proc1(void){
	int i, status;
	status = 1;
	
	set_process_priority(gp_current_process->m_pid, HIGH);
	for(i = 0; i < 30; i++) {
		all_memory_blocks[i] = request_memory_block();
	}
	set_process_priority(gp_current_process->m_pid, LOWEST);
	
	for(i = 0; i < 30; i++) {
		status &= (release_memory_block(all_memory_blocks[i]) == RTX_OK) ? 1 : 0;
	}
	
	printTest(1, status);
	checkTestEnd();
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Test which ensures that we can get a memory block properly.
void proc2(void){
	void *memory_block = request_memory_block();
	printTest(2, 1);
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

// Test which ensures that we correctly set priorities.
void proc5(void){
	int status, priority;
	set_process_priority(gp_current_process->m_pid, MEDIUM);
	priority = get_process_priority(gp_current_process->m_pid);
	status = priority == MEDIUM ? 1 : 0;
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
