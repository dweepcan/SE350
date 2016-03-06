#include "rtx.h"
#include "sys_proc.h"
#include "timer.h"
#include "uart.h"
#include "k_memory.h"


#define NUM_COMMANDS 25
#define CMD_LENGTH 50

typedef struct kcd_command{
	char command[CMD_LENGTH]; //command identifier 
	int pid;
} kcd_command;

kcd_command kcd_commands[NUM_COMMANDS];
char actualMsg [BLOCK_SIZE - sizeof(MSG_BUF)];

/* initialization table item */
PROC_INIT g_sys_procs[NUM_SYS_PROCS];

void set_sys_procs() {
	g_sys_procs[0].m_pid=(U32)(0);
	g_sys_procs[0].m_priority=SYS_NULL_PRIORITY;
	g_sys_procs[0].m_stack_size=0x100;
 	g_sys_procs[0].mpf_start_pc = &proc_null;
	
	// TODO: change the priority to make sense
	g_sys_procs[1].m_pid=(U32)PID_KCD;
	g_sys_procs[1].m_priority=SYS_HIGHEST;
	g_sys_procs[1].m_stack_size=0x100;
 	g_sys_procs[1].mpf_start_pc = &proc_kcd;
	
	g_sys_procs[2].m_pid=(U32)PID_CRT;
	g_sys_procs[2].m_priority=SYS_HIGHEST;
	g_sys_procs[2].m_stack_size=0x100;
 	g_sys_procs[2].mpf_start_pc = &proc_crt;
	
	g_sys_procs[3].m_pid=(U32)PID_TIMER_IPROC;
	g_sys_procs[3].m_priority=SYS_I_PROC;
	g_sys_procs[3].m_stack_size=0x100;
 	g_sys_procs[3].mpf_start_pc = &proc_null; // so we know if we messed up
	
	g_sys_procs[4].m_pid=(U32)PID_UART_IPROC;
	g_sys_procs[4].m_priority=SYS_I_PROC;
	g_sys_procs[4].m_stack_size=0x100;
 	g_sys_procs[4].mpf_start_pc = &proc_null; // so we know if we messed up
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
	
	if (*s=='\0') return 1;
	
	return 0;
}

void copyString(char* s,char* t){
	while (*s!='\0' && *s!='\r' && *s!='\n'){
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
					
					k_release_memory_block((void *)msg);
				}
			} else if (msg->mtype == DEFAULT){
	
				//always send the message to crt - see page 13 of manual 
				copyString(msg->mtext,actualMsg);
 				//k_release_memory_block(msg);							
 				msg->mtype = CRT_DISP;
 				send_message(PID_CRT,(void *)msg);
				
				//execute command
				if (actualMsg[0]=='%'){	
					for (i=0; i<numStoredCommands; i++){
						if (compareCmd(kcd_commands[i].command, actualMsg) == 1){
								//Send registered command to its mapped process
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

void proc_crt(void){
	MSG_BUF* msg;
	int pid;
	
	while(1){
		msg = (MSG_BUF*)receive_message(&pid);
		if (msg != NULL && msg->mtype == CRT_DISP){
			//Send messages of CRT Display type to the uart for display
			msg->mtype = DEFAULT;
			send_message(PID_UART_IPROC, (void*) msg);
			
			triggerUart();
		}else{
			k_release_memory_block((void *)msg);
		}
	}
}

