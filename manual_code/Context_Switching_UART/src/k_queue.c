#include "k_queue.h"

//#ifdef DEBUG_0
#include "printf.h"
//#endif /* ! DEBUG_0 */

int enqueue(k_queue *p_queue, k_node *p_node) {
    if(p_queue == NULL || p_node == NULL) {
#ifdef DEBUG_0
		printf("ERROR: Enqueuing to a NULL queue or inserting a NULL node.\n");
#endif /* ! DEBUG_0 */
        return RTX_ERR;
    }

    p_node->next = NULL;
    if(q_is_empty(p_queue)) {
        p_queue->first = p_node;
    } else {
        p_queue->last->next = p_node;
    }

    p_queue->last = p_node;

    return RTX_OK;
}

k_node* dequeue(k_queue *p_queue) {
    k_node *p_first = NULL;

    if (q_is_empty(p_queue)) {
#ifdef DEBUG_0
        printf("ERROR: Dequeuing a node from an empty queue.\n");
#endif /* ! DEBUG_0 */
        return NULL;
    }

    p_first = p_queue->first;

    if(p_queue->first == p_queue->last) {
        p_queue->last = NULL;
    }

    p_queue->first = p_queue->first->next;

    return p_first;
}

int q_is_empty(k_queue *p_queue) {
    if(p_queue == NULL) {
#ifdef DEBUG_0
        printf("ERROR: Checking if a NULL queue is empty.\n");
#endif /* ! DEBUG_0 */
		return 0;
    } else {
        return (p_queue->first == NULL);
    }
}
