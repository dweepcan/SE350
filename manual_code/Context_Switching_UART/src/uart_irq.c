/**
 * @brief: uart_irq.c 
 * @author: NXP Semiconductors
 * @author: Y. Huang
 * @date: 2014/02/28
 */

#include <LPC17xx.h>
#include "uart.h"
#include "uart_polling.h"
#include "k_memory.h"
#include "k_process.h"
#include "k_rtx.h"
//#ifdef DEBUG_0
#include "printf.h"
//#endif

#ifdef _DEBUG_HOTKEYS
#define READY_HK '!'
#define BLOCKED_RESOURCE_HK '@'
#define BLOCKED_RECEIVE_HK '#'
#define MEMORY_HEAP_HK '$'
#endif

uint8_t g_buffer[MSG_BUF_TEXT_SIZE];
uint8_t *gp_buffer = g_buffer;
uint8_t g_send_char = 0;
uint8_t g_char_in;
uint8_t g_char_out;

extern uint32_t g_switch_flag;

char stringBuilder[MSG_BUF_TEXT_SIZE];
int stringCurrentIndex = 0;

//extern int k_release_processor(void);
/**
 * @brief: initialize the n_uart
 * NOTES: It only supports UART0. It can be easily extended to support UART1 IRQ.
 * The step number in the comments matches the item number in Section 14.1 on pg 298
 * of LPC17xx_UM
 */

void copyStringAddNewLine(char* s,char* t){
	while (*s!='\0'){
		*t = *s;
		s=s+1;
		t=t+1;
	}
	
	*t='\0';
}

int uart_irq_init(int n_uart) {

	LPC_UART_TypeDef *pUart;

	if ( n_uart ==0 ) {
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
		//LPC_SC->PCONP |= BIT(3);
	
		-----------------------------------------------------
		Step2: Select the clock source. 
		       Default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ 
		gets to CCLK=100MHZin system_LPC17xx.c file.
		PCLK = CCLK/4, default setting after reset.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(7)|BIT(6));	
			
		-----------------------------------------------------
		Step 5: Pin Ctrl Block configuration for TXD and RXD
		        See Table 79 on pg108 in LPC17xx_UM.
		-----------------------------------------------------
		Note this is done before Steps3-4 for coding purpose.
		*/
		
		/* Pin P0.2 used as TXD0 (Com0) */
		LPC_PINCON->PINSEL0 |= (1 << 4);  
		
		/* Pin P0.3 used as RXD0 (Com0) */
		LPC_PINCON->PINSEL0 |= (1 << 6);  

		pUart = (LPC_UART_TypeDef *) LPC_UART0;	 
		
	} else if ( n_uart == 1) {
	    
		/* see Table 79 on pg108 in LPC17xx_UM */ 
		/* Pin P2.0 used as TXD1 (Com1) */
		LPC_PINCON->PINSEL4 |= (2 << 0);

		/* Pin P2.1 used as RXD1 (Com1) */
		LPC_PINCON->PINSEL4 |= (2 << 2);	      

		pUart = (LPC_UART_TypeDef *) LPC_UART1;
		
	} else {
		return 1; /* not supported yet */
	} 
	
	/*
	-----------------------------------------------------
	Step 3: Transmission Configuration.
	        See section 14.4.12.1 pg313-315 in LPC17xx_UM 
	        for baud rate calculation.
	-----------------------------------------------------
        */
	
	/* Step 3a: DLAB=1, 8N1 */
	pUart->LCR = UART_8N1; /* see uart.h file */ 

	/* Step 3b: 115200 baud rate @ 25.0 MHZ PCLK */
	pUart->DLM = 0; /* see table 274, pg302 in LPC17xx_UM */
	pUart->DLL = 9;	/* see table 273, pg302 in LPC17xx_UM */
	
	/* FR = 1.507 ~ 1/2, DivAddVal = 1, MulVal = 2
	   FR = 1.507 = 25MHZ/(16*9*115200)
	   see table 285 on pg312 in LPC_17xxUM
	*/
	pUart->FDR = 0x21;       
	
 

	/*
	----------------------------------------------------- 
	Step 4: FIFO setup.
	       see table 278 on pg305 in LPC17xx_UM
	-----------------------------------------------------
        enable Rx and Tx FIFOs, clear Rx and Tx FIFOs
	Trigger level 0 (1 char per interrupt)
	*/
	
	pUart->FCR = 0x07;

	/* Step 5 was done between step 2 and step 4 a few lines above */

	/*
	----------------------------------------------------- 
	Step 6 Interrupt setting and enabling
	-----------------------------------------------------
	*/
	/* Step 6a: 
	   Enable interrupt bit(s) wihtin the specific peripheral register.
           Interrupt Sources Setting: RBR, THRE or RX Line Stats
	   See Table 50 on pg73 in LPC17xx_UM for all possible UART0 interrupt sources
	   See Table 275 on pg 302 in LPC17xx_UM for IER setting 
	*/
	/* disable the Divisior Latch Access Bit DLAB=0 */
	pUart->LCR &= ~(BIT(7)); 
	
	//pUart->IER = IER_RBR | IER_THRE | IER_RLS; 
	pUart->IER = IER_RBR | IER_RLS;

	/* Step 6b: enable the UART interrupt from the system level */
	
	if ( n_uart == 0 ) {
		NVIC_EnableIRQ(UART0_IRQn); /* CMSIS function */
	} else if ( n_uart == 1 ) {
		NVIC_EnableIRQ(UART1_IRQn); /* CMSIS function */
	} else {
		return 1; /* not supported yet */
	}
	pUart->THR = '\0';
	return 0;
}


/**
 * @brief: use CMSIS ISR for UART0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_UART0_IRQHandler does the rest of irq handling
 */
__asm void UART0_IRQHandler(void)
{
	PRESERVE8
	IMPORT c_UART0_IRQHandler
	IMPORT k_release_processor
	PUSH{r4-r11, lr}
	BL c_UART0_IRQHandler
	LDR R4, =__cpp(&g_switch_flag)
	LDR R4, [R4]
	MOV R5, #0     
	CMP R4, R5
	BEQ  RESTORE    ; if g_switch_flag == 0, then restore the process that was interrupted
	BL k_release_processor  ; otherwise (i.e g_switch_flag == 1, then switch to the other process)
RESTORE
	POP{r4-r11, pc}
} 

/**
 * @brief: c UART0 IRQ Handler
 */
void c_UART0_IRQHandler(void)
{
	int i;
	uint8_t IIR_IntId;	    // Interrupt ID from IIR 		 
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
		
	__disable_irq();

#ifdef DEBUG_0
	uart1_put_string("Entering c_UART0_IRQHandler\n\r");
#endif // DEBUG_0
	
	g_switch_flag = 0;

	/* Reading IIR automatically acknowledges the interrupt */
	IIR_IntId = (pUart->IIR) >> 1 ; // skip pending bit in IIR 
	if (IIR_IntId & IIR_RDA) { // Receive Data Avaialbe
		/* read UART. Read RBR will clear the interrupt */
		g_char_in = pUart->RBR;
#ifdef DEBUG_0
		uart1_put_string("Reading a char = ");
		uart1_put_char(g_char_in);
		uart1_put_string("\n\r");
#endif // DEBUG_0
		// Prints the character to UART
		kcd_helper(g_char_in);
	} else if (IIR_IntId & IIR_THRE) {
		
	/* THRE Interrupt, transmit holding register becomes empty */
STUPIDLABEL:
		if (*gp_buffer != '\0' ) {
			g_char_out = *gp_buffer;
#ifdef DEBUG_0
			//uart1_put_string("Writing a char = ");
			//uart1_put_char(g_char_out);
			//uart1_put_string("\n\r");
			
			// you could use the printf instead
			printf("Writing a char = %c \n\r", g_char_out);
#endif // DEBUG_0			
			pUart->THR = g_char_out;
			gp_buffer++;
		} else {
			if(crt_helper() ==0){

	#ifdef DEBUG_0
				uart1_put_string("Finish writing. Turning off IER_THRE\n\r");
	#endif // DEBUG_0
				pUart->IER ^= IER_THRE; // toggle the IER_THRE bit 
				pUart->THR = '\0';
				g_send_char = 0;
				
				for (i=0; i<MSG_BUF_TEXT_SIZE; i++){
					g_buffer[i] = '\0';
				}
			} else{
				gp_buffer = g_buffer;
				goto STUPIDLABEL;
			}
		}
	      
	} else {  /* not implemented yet */
#ifdef DEBUG_0
			uart1_put_string("Should not get here!\n\r");
#endif // DEBUG_0
		__enable_irq();
		return;
	}	
	__enable_irq();
}

void kcd_helper(uint8_t char_in){
	MSG_BUF *p_msg_env;
	int i;
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	
#ifdef _DEBUG_HOTKEYS
	if (char_in == READY_HK){
		printQueue(RDY);
	}else if (char_in == BLOCKED_RESOURCE_HK){
		printQueue(BLOCKED_ON_RESOURCE);
	}else if (char_in == BLOCKED_RECEIVE_HK){
		printQueue(BLOCKED_ON_RECEIVE);
	}else if (char_in == MEMORY_HEAP_HK) {
		printMemoryHeap();
  }
#endif
	
	if(char_in != '\r' && stringCurrentIndex < MSG_BUF_TEXT_SIZE - 3) {
#ifdef DEBUG_0
		printf("Received %c\n\r", (char) char_in);
#endif
		// Need this hack because you can only cast the u_int_8 pointer to char * pointer
		pUart->THR = char_in;

		stringBuilder[stringCurrentIndex++] = *((char *) &char_in);		
	} else {
#ifdef DEBUG_0
		printf("Received carriage return character\n\r");
#endif
		
		pUart->THR = '\r';
		pUart->THR = '\n';
		pUart->THR = '\0';
		
		p_msg_env = (MSG_BUF *) k_request_memory_block_nonblocking();	
		if(p_msg_env!= NULL) {
			stringBuilder[stringCurrentIndex++] = '\r';
			stringBuilder[stringCurrentIndex++] = '\n';
			stringBuilder[stringCurrentIndex++] = '\0';
				
			p_msg_env->m_send_pid = PID_UART_IPROC;
			p_msg_env->m_recv_pid = PID_KCD;
			p_msg_env->mp_next = NULL;
			p_msg_env->mtype = DEFAULT;
			
			for (i=0; i<stringCurrentIndex; i++){
				p_msg_env->mtext[i] = stringBuilder[i];
			}
			
			k_send_message_nonblocking(PID_KCD, p_msg_env);
			
			g_switch_flag = 1;
		}
		
		stringCurrentIndex = 0;
	}
}



int crt_helper(){
	MSG_BUF* msg;
	//int pid;
	PCB *temp = gp_current_process;
	gp_current_process = processNodes[PID_UART_IPROC]->pcb;
	
	msg = (MSG_BUF*)k_receive_message_nonblocking(NULL);
	gp_current_process = temp;

	if(msg != NULL){
		copyStringAddNewLine(msg->mtext,(char *)g_buffer);
		if (k_release_memory_block_nonblocking(msg) == 1){
			g_switch_flag = 1;
		}
		
		return 1; // 1 for received message
	}
	
	// k_release_memory_block_nonblocking(msg);
	return 0; //0 for no message
}

void triggerUart(){
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	pUart->IER |= IER_THRE; // see line 151 of uart_irq.c
}
