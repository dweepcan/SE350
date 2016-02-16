#include "rtx.h"
#include "sys_proc.h"

/* initialization table item */
PROC_INIT g_sys_procs[NUM_SYS_PROCS];

void set_sys_procs() {
	g_sys_procs[0].m_pid=(U32)(0);
	g_sys_procs[0].m_priority=LOWEST+1;
	g_sys_procs[0].m_stack_size=0x100;
 	g_sys_procs[0].mpf_start_pc = &proc_null;
}

//The null process
void proc_null(void) {
	while(1) {
		release_processor();
	}
}
