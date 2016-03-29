/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "k_process.h"

extern Queue *readyPriorityQueue[NUM_PRIORITIES];

#define BIT(X) (1<<X)

volatile U32 g_timer_count = 0; // increment every 1 ms
volatile U32* primitive_timer_count; 

// This timer ticks every 0.4 micro seconds
#define PRIMITIVE_TIMER_TICK  4;

k_msg_queue *pendingMessageQueue;

/**
 * @brief: initialize timer. Only timer 0 is supported
 */
U32 timer_init(U8 n_timer) 
{
	LPC_TIM_TypeDef *pTimer;
	if (n_timer == 0) {
		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps
		
		----------------------------------------------------- 
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(1);
	
		-----------------------------------------------------
		Step2: Select the clock source, 
		       default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ 
		gets to CCLK=100MHZ in system_LPC17xx.c file.
		PCLK = CCLK/4, default setting in system_LPC17xx.c.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));	

		-----------------------------------------------------
		Step 3: Pin Ctrl Block configuration. 
		        Optional, not used in this example
		        See Table 82 on pg110 in LPC17xx_UM 
		-----------------------------------------------------
		*/
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM0;
			
			
		/*
		-----------------------------------------------------
		Step 4: Interrupts configuration
		-----------------------------------------------------
		*/

		/* Step 4.1: Prescale Register PR setting 
			 CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
			 2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
			 TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
			 see MR setting below 
		*/
		pTimer->PR = 12499;  

		/* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
		pTimer->MR0 = 1;

		/* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
			 Interrupt on MR0: when MR0 mathches the value in the TC, 
												 generate an interrupt.
			 Reset on MR0: Reset TC if MR0 mathches it.
		*/
		pTimer->MCR = BIT(0) | BIT(1);

		g_timer_count = 0;

		/* Step 4.4: CSMSIS enable timer0 IRQ */
		NVIC_EnableIRQ(TIMER0_IRQn);

		/* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
		pTimer->TCR = 1;

	} else if (n_timer == 1){
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM1;
		
		pTimer->PR = PRIMITIVE_TIMER_TICK;
		pTimer->MCR = BIT(0) | BIT(1);
		pTimer->TCR = 1;
		pTimer->TC = 1;		
		
		primitive_timer_count = &(pTimer->TC);
		
	} else { /* other timer not supported yet */
		return 1;
	}


	return 0;
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void)
{
	PRESERVE8
	IMPORT c_TIMER0_IRQHandler
	PUSH{r4-r11, lr}
	BL c_TIMER0_IRQHandler
	POP{r4-r11, pc}
} 
/**
 * @brief: c TIMER0 IRQ Handler
 */
void c_TIMER0_IRQHandler(void)
{
	__disable_irq();
	
	/* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
	LPC_TIM0->IR = BIT(0);  
	
	g_timer_count++ ;
	
	//context switch set current to timer 
	timer_i_process();
	
	__enable_irq();
}

// Function which sets up the message queue
void pending_message_queue_init(void) {
	pendingMessageQueue -> size = 0;
	pendingMessageQueue -> first = NULL;
	pendingMessageQueue -> last = NULL;
}

void timer_i_process(){
	int i = 0;
	MSG_BUF* msg;
	int target_pid;
	
	while(pendingMessageQueue->first != NULL) {
		if((U32)(pendingMessageQueue->first->m_kdata[0])<=g_timer_count) {
			msg = msg_dequeue(pendingMessageQueue);
			target_pid = msg->m_recv_pid;
			msg->mp_next = NULL;
			k_send_message_nonblocking(target_pid, (void*)msg);
		} else {
			break;
		}
	}
	
	for (i=0;i<NUM_PRIORITIES;i++){
			if (readyPriorityQueue[i]->front !=NULL && readyPriorityQueue[i]->front->pcb->m_priority < gp_current_process->m_priority){
				__enable_irq();
				k_release_processor();
				__disable_irq();
				break;
			}
	}
}
