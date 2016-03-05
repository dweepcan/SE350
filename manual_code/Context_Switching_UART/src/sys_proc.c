#include "rtx.h"
#include "sys_proc.h"
#include "timer.h"
#include "uart.h"
#include "k_memory.h"


#define NUM_COMMANDS 25
#define CMD_LENGTH 25

typedef struct{
	char command[CMD_LENGTH]; //command identifier 
	int pid;
} kcd_command;

kcd_command kcd_commands[NUM_COMMANDS];
char actualMsg [BLOCK_SIZE - sizeof(MSG_BUF)];

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
 	g_sys_procs[2].mpf_start_pc = &timer_i_process;
	
	g_sys_procs[3].m_pid=(U32)PID_UART_IPROC;
	g_sys_procs[3].m_priority=I_PROC;
	g_sys_procs[3].m_stack_size=0x100;
 	g_sys_procs[3].mpf_start_pc = &uart_i_process;
}

//The null process
void proc_null(void) {
	while(1) {
		release_processor();
	}
}




int compareCmd(char* s, char* t){
	while(*s!='\0' && *t!='\0'){
		if (*s != *t) return 0;
 		s = s+1;
 		t = t+1;		
	}
	
	if (*s=='\0' && *t=='\0') return 1;
	
	if (*s=='\0' && *t==' ') return 1;
	
	return 0;
	// 	while(*s!='\0' && !(*t==' '|| *t=='\0')){
// 		if  (*t==' '||*t=='\0')  return -1;
// 		if (*s != *t) return -1;
// 		s = s+1;
// 		t = t+1;
// 	}
// 	return 1;
}

void copyString(char* s,char* t){
	while (*s!='\0'){
		*t = *s;
		s=s+1;
		t=t+1;
	}
	*t='\0';
}

// Keyboard Command Decoder Process
void proc_kcd(void) {
	
	int numStoredCommands = 0;
	MSG_BUF* msg;
	int pid;
	int i;
	
	while(1){
		msg = (MSG_BUF*)receive_message(&pid);
		if (msg != NULL){
			if (msg->mtype == KCD_REG){
				//if its a command process it, otherwise dont do anything
				if (numStoredCommands < NUM_COMMANDS){
					copyString(msg->mtext,kcd_commands[numStoredCommands].command);
					kcd_commands[numStoredCommands].pid = pid;
					numStoredCommands++;
					k_release_memory_block_nonblocking(msg);
				}
			}else if (msg->mtype == DEFAULT){
				//execute command
				if (msg->mtext[0]=='%'){
					for (i=0; i<numStoredCommands; i++){
						if (compareCmd(kcd_commands[i].command, msg->mtext) == 1){
								//do send message
								
								copyString(msg->mtext,actualMsg);
// 								k_release_memory_block_nonblocking(msg);							
// 								msg = (MSG_BUF *)request_memory_block();
// 								msg->mtype = DEFAULT;
// 								copyString(actualMsg, msg->mtext);
// 								send_message(PID_CRT,(void *)msg);			
							
								msg = (MSG_BUF *)request_memory_block();
								msg->mtype = DEFAULT;
								copyString(actualMsg, msg->mtext);
								send_message(kcd_commands[i].pid,(void *)msg);			
						}
					}
				}			
			}
		}
	}
}

