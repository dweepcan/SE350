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

void proc1(void){
	int i = 0;
	printf("Entering process 1.\r\n");
	i += set_process_priority(PID_NULL, HIGH);
	i += set_process_priority(PID_TIMER_IPROC, LOW);
	i += set_process_priority(gp_current_process->m_pid, -1);
	i += set_process_priority(gp_current_process->m_pid, 4);
	if(i==-4) {
		printf("YAY.\r\n");
	} else {
		printf("NO.\r\n");
	}
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc2(void){
	printf("Entering process 2.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc3(void){
	printf("Entering process 3.\r\n");
	while(1) {
		release_processor();
	}
}

void proc4(void){
	printf("Entering process 4.\r\n");
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

void proc5(void){
	printf("Entering process 5.\r\n");
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
