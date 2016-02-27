#ifndef K_QUEUE_H
#define K_QUEUE_H

#include "k_rtx.h"
#include "k_node.h"

typedef struct k_queue {
    k_node *first;
    k_node *last;
} k_queue;

int msg_enqueue(k_queue *p_queue, k_node *p_node);
k_node* msg_dequeue(k_queue *p_queue);
int msg_q_is_empty(k_queue *p_queue);

#endif //K_QUEUE_H
