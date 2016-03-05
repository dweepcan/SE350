#include "defined_user_proc.h"
#include "rtx.h"
#include "k_memory.h"

PROC_INIT g_user_procs[NUM_USER_PROCS];
void set_user_procs(void){
	g_user_procs[0].m_pid=(U32)(PID_A);
	g_user_procs[0].m_priority=LOWEST+1;
	g_user_procs[0].m_stack_size=0x100;
 	g_user_procs[0].mpf_start_pc = &stress_test_a;
	
	g_user_procs[1].m_pid=(U32)(PID_B);
	g_user_procs[1].m_priority=LOWEST+1;
	g_user_procs[1].m_stack_size=0x100;
 	g_user_procs[1].mpf_start_pc = &stress_test_b;
	
	g_user_procs[2].m_pid=(U32)(PID_C);
	g_user_procs[2].m_priority=LOWEST+1;
	g_user_procs[2].m_stack_size=0x100;
 	g_user_procs[2].mpf_start_pc = &stress_test_c;
	
	g_user_procs[3].m_pid=(U32)(PID_SET_PRIO);
	g_user_procs[3].m_priority=LOWEST+1;
	g_user_procs[3].m_stack_size=0x100;
 	g_user_procs[3].mpf_start_pc = &set_priority_command;
	
	g_user_procs[4].m_pid=(U32)(PID_CLOCK);
	g_user_procs[4].m_priority=LOWEST+1;
	g_user_procs[4].m_stack_size=0x100;
 	g_user_procs[4].mpf_start_pc = &wall_clock;
}

void wall_clock(){
	while(1) {
		release_processor();
	}
}

//TODO Lab 3
void set_priority_command(){
	while(1) {
		release_processor();
	}
}	
void stress_test_a(){
	while(1) {
		release_processor();
	}
}
void stress_test_b(){
	while(1) {
		release_processor();
	}
}
void stress_test_c(){
	while(1) {
		release_processor();
	}
}
