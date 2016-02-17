// A test suite to ensure that the null process can run properly when every other process is blocked
// NOTE: NEEDS TO BE RUN WITH ONLY ONE MEMORY BLOCK AVAILABLE

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	
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

// Request more than the available memory blocks, this will cause everything else to block upon request, including itself
void proc1(void){
	void *memory_block;
	void *memory_block2;
	set_process_priority(gp_current_process->m_pid, HIGH);
	memory_block = request_memory_block();
	memory_block2 = request_memory_block();
	
	// unreachable code
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Request a single, unavailble memory block and be immediately blocked
void proc2(void){
	void *memory_block = request_memory_block();
	
	// unreachable code
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Request a single, unavailble memory block and be immediately blocked
void proc3(void){
	void *memory_block = request_memory_block();
	
	// unreachable code
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Request a single, unavailble memory block and be immediately blocked
void proc4(void){
	void *memory_block = request_memory_block();
	
	// unreachable code
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Request a single, unavailble memory block and be immediately blocked
void proc5(void){
	void *memory_block = request_memory_block();
	
	// unreachable code
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}

// Request a single, unavailble memory block and be immediately blocked
void proc6(void){
	void *memory_block = request_memory_block();
	
	// unreachable code
	set_process_priority(gp_current_process->m_pid, LOWEST);
	while(1) {
		release_processor();
	}
}
