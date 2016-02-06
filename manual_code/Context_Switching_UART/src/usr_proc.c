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

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS+1];//plus last one nullproc

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
		g_test_procs[NUM_TEST_PROCS].m_pid=(U32)(0);
		g_test_procs[NUM_TEST_PROCS].m_priority=LOWEST+1;
		g_test_procs[NUM_TEST_PROCS].m_stack_size=0x100;
	
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[5].mpf_start_pc = &proc6;
}

// Helper function to print out end test results
void printTest(int testNum, int status) {
	sprintf(buffer, "G012_test: test %d %s", testNum, status==0 ? "FAIL" : "OK");
	uart1_put_string((unsigned char*) buffer);
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


//The null process
//uart0_put_string("Hello World");
void proc_null(void) {
	while(1) {
		k_release_processor();
	}
}


// TODO: CHANGE THESE TWO PIECES OF SHIT
/**
 * @brief: a process that prints 5x6 uppercase letters
 *         and then yields the cpu.
 */
void proc1(void)
{
	void *memory_block = k_request_memory_block();
	printTest(1, 1);
	checkTestEnd();
	while(1) {
		k_release_processor();
	}
	
	/*
	int i = 0;
	int ret_val = 10;
	int x = 0;

	
	
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart1_put_string("\n\r");
			
			if ( i%30 == 0 ) {
				ret_val = release_processor();
	
#ifdef DEBUG_0
				printf("proc1: ret_val=%d\n", ret_val);
			
#endif
			}
			for ( x = 0; x < 500000; x++); // some artifical delay
		}
		uart1_put_char('A' + i%26);
		i++;
		
	}
	*/
}

/**
 * @brief: a process that prints 5x6 numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	void *memory_block = k_request_memory_block();
	printTest(2, 1);
	checkTestEnd();
	while(1) {
		k_release_processor();
	}
	
	/*
	int i = 0;
	int ret_val = 20;
	int x = 0;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart1_put_string("\n\r");
			
			if ( i%30 == 0 ) {
				ret_val = release_processor();

#ifdef DEBUG_0
				printf("proc2: ret_val=%d\n", ret_val);
			
#endif
			}
			for ( x = 0; x < 500000; x++); // some artifical delay
		}
		uart1_put_char('0' + i%10);
		i++;
		
	}
	*/
}

void proc3(void){
	void *memory_block = k_request_memory_block();
	printTest(3, 1);
	checkTestEnd();
	while(1) {
		k_release_processor();
	}
}

void proc4(void){
	void *memory_block = k_request_memory_block();
	printTest(4, 1);
	checkTestEnd();
	while(1) {
		k_release_processor();
	}
}

void proc5(void){
	void *memory_block = k_request_memory_block();
	printTest(5, 1);
	checkTestEnd();
	while(1) {
		k_release_processor();
	}
}

void proc6(void){
	void *memory_block = k_request_memory_block();
	printTest(6, 1);
	checkTestEnd();
	while(1) {
		k_release_processor();
	}
}
