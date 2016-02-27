#ifndef K_MSG_QUEUE_H
#define K_MSG_QUEUE_H

typedef struct msgbuf MSG_BUF;

typedef struct k_msg_queue {
    MSG_BUF *first;
    MSG_BUF *last;
} k_msg_queue;

int msg_enqueue(k_msg_queue *p_queue, MSG_BUF *p_node);
MSG_BUF* msg_dequeue(k_msg_queue *p_queue);
int msg_q_is_empty(k_msg_queue *p_queue);

#endif //K_MSG_QUEUE_H
