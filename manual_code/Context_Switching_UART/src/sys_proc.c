#include "rtx.h"
#include "sys_proc.h"
#include "timer.h"
#include "uart.h"

/* initialization table item */
PROC_INIT g_sys_procs[NUM_SYS_PROCS];

void set_sys_procs() {
	g_sys_procs[0].m_pid=(U32)(0);
	g_sys_procs[0].m_priority=LOWEST+1;
	g_sys_procs[0].m_stack_size=0x100;
 	g_sys_procs[0].mpf_start_pc = &proc_null;
	
	// TODO: change the priority to make sense
	g_sys_procs[1].m_pid=(U32)PID_KCD;
	g_sys_procs[1].m_priority=HIGHEST;
	g_sys_procs[1].m_stack_size=0x100;
 	g_sys_procs[1].mpf_start_pc = &proc_kcd;
	
	g_sys_procs[2].m_pid=(U32)PID_TIMER_IPROC;
	g_sys_procs[2].m_priority=I_PROC;
	g_sys_procs[2].m_stack_size=0x100;
 	g_sys_procs[2].mpf_start_pc = &proc_null; // so we know if we messed up
	
	g_sys_procs[3].m_pid=(U32)PID_UART_IPROC;
	g_sys_procs[3].m_priority=I_PROC;
	g_sys_procs[3].m_stack_size=0x100;
 	g_sys_procs[3].mpf_start_pc = &proc_null; // so we know if we messed up
}

//The null process
void proc_null(void) {
	while(1) {
		release_processor();
	}
}

// Keyboard Command Decoder Process
void proc_kcd(void) {
	receive_message(NULL);
}
