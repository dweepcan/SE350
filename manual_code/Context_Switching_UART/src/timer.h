/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include "k_rtx.h"
#include "k_message.h"
#include "k_msg_queue.h"

extern U32 timer_init ( U8 n_timer );  /* initialize timer n_timer */
extern void pending_message_queue_init(void);
void timer_i_process(void);

#endif /* ! _TIMER_H_ */
