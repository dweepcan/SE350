#include "defined_user_proc.h"
#include "rtx.h"
#include "k_memory.h"

PROC_INIT g_user_procs[NUM_USER_PROCS];
void set_user_procs(void){
	g_user_procs[0].m_pid=(U32)(PID_A);
	g_user_procs[0].m_priority=SYS_NULL_PRIORITY;
	g_user_procs[0].m_stack_size=0x100;
 	g_user_procs[0].mpf_start_pc = &stress_test_a;
	
	g_user_procs[1].m_pid=(U32)(PID_B);
	g_user_procs[1].m_priority=SYS_NULL_PRIORITY;
	g_user_procs[1].m_stack_size=0x100;
 	g_user_procs[1].mpf_start_pc = &stress_test_b;
	
	g_user_procs[2].m_pid=(U32)(PID_C);
	g_user_procs[2].m_priority=SYS_NULL_PRIORITY;
	g_user_procs[2].m_stack_size=0x100;
 	g_user_procs[2].mpf_start_pc = &stress_test_c;
	
	g_user_procs[3].m_pid=(U32)(PID_SET_PRIO);
	g_user_procs[3].m_priority=SYS_NULL_PRIORITY;
	g_user_procs[3].m_stack_size=0x100;
 	g_user_procs[3].mpf_start_pc = &set_priority_command;
	
	g_user_procs[4].m_pid=(U32)(PID_CLOCK);
	g_user_procs[4].m_priority=SYS_HIGHEST;
	g_user_procs[4].m_stack_size=0x100;
 	g_user_procs[4].mpf_start_pc = &wall_clock;
}

void wall_clock(){
	MSG_BUF *msg;
	int time;
	int pid;
	int active;
	
	msg = (MSG_BUF *)request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = '%';
	msg->mtext[1] = 'W';
	msg->mtext[2] = '\0';
	send_message(PID_KCD, (void *)msg);
	
	time = 0;
	active = 1;
	
	while(1){
		msg = (MSG_BUF *)receive_message(&pid);
		
		if (pid == PID_CLOCK && active == 1){
		
			msg->mtype = DEFAULT;
			delayed_send(PID_CLOCK,(void *)msg, 100); //1000 millisecond delay?
			
			msg = (MSG_BUF *) request_memory_block();
			msg->mtype = CRT_DISP;
			msg->mtext[0] = (char)((time/3600)%24/10 + '0');
			msg->mtext[1] = (char)((time/3600)%24%10 + '0');
			msg->mtext[2] = ':';
			msg->mtext[3] = (time/60)%60/10 +'0';
			msg->mtext[4] = (time/60)%60%10 +'0';
			msg->mtext[5] = ':';
			msg->mtext[6] = (time%60)/10 + '0';
			msg->mtext[7] = (char)(time%60%10 + '0');
			msg->mtext[8] = '\r';
			msg->mtext[9] = '\n';
			msg->mtext[10] = '\0';

			

			send_message(PID_CRT, msg);
			time++;
			time = time%(60*60*24);
		}else{
			if (msg->mtext[2]== 'R'){ //reset clock
				time = 0; //should i do more checks?
			}else if (msg->mtext[2] == 'T'){ //terminate command just quit bruh
				break;
			
			}else if (msg->mtext[2] == 'S'){ //set time
			}else if (msg->mtext[1] == 'W'){ //cheat to start clock...
				//msg = (MSG_BUF *) request_memory_block();
				msg->mtype = DEFAULT;
				send_message(PID_CLOCK, msg);
				active = 1;
			}
		}
	}
	
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
