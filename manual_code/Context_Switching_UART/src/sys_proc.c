#include "rtx.h"
#include "sys_proc.h"
#include "timer.h"

/* initialization table item */
PROC_INIT g_sys_procs[NUM_SYS_PROCS];

void set_sys_procs() {
	g_sys_procs[0].m_pid=(U32)(0);
	g_sys_procs[0].m_priority=LOWEST+1;
	g_sys_procs[0].m_stack_size=0x100;
 	g_sys_procs[0].mpf_start_pc = &proc_null;
	
	g_sys_procs[1].m_pid=(U32)(7);
	g_sys_procs[1].m_priority=I_PROC;
	g_sys_procs[1].m_stack_size=0x100;
 	g_sys_procs[1].mpf_start_pc = &timer_i_process;
}

//The null process
void proc_null(void) {
	while(1) {
		release_processor();
	}
}
