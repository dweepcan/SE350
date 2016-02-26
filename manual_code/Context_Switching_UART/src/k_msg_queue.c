#include "k_msg_queue.h"
#include "k_rtx.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

int enqueue(k_msg_queue *p_queue, MSG_BUF *p_node) {
    if(p_queue == NULL || p_node == NULL) {
#ifdef DEBUG_0
		printf("ERROR: Enqueuing to a NULL queue or inserting a NULL node.\n");
#endif /* ! DEBUG_0 */
        return RTX_ERR;
    }

    p_node->mp_next = NULL;
    if(q_is_empty(p_queue)) {
        p_queue->first = p_node;
    } else {
        p_queue->last->mp_next = p_node;
    }

    p_queue->last = p_node;

    return RTX_OK;
}

MSG_BUF* dequeue(k_msg_queue *p_queue) {
    MSG_BUF *p_first = NULL;

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

    p_queue->first = p_queue->first->mp_next;

    return p_first;
}

int q_is_empty(k_msg_queue *p_queue) {
    if(p_queue == NULL) {
#ifdef DEBUG_0
        printf("ERROR: Checking if a NULL queue is empty.\n");
#endif /* ! DEBUG_0 */
		return 0;
    } else {
        return (p_queue->first == NULL);
    }
}
