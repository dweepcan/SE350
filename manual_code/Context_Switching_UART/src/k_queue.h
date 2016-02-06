#ifndef K_QUEUE_H
#define K_QUEUE_H

#include "k_rtx.h"
#include "k_node.h"

typedef struct k_queue {
    k_node *first;
    k_node *last;
} k_queue;

void enqueue(k_queue *p_queue, k_node *p_node);
k_node* dequeue(k_queue *p_queue);
int is_empty(k_queue *p_queue);

#endif //K_QUEUE_H
