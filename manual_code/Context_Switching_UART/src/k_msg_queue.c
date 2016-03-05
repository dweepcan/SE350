#include "k_msg_queue.h"
#include "k_rtx.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

int msg_enqueue(k_msg_queue *p_queue, MSG_BUF *p_node) {
    if(p_queue == NULL || p_node == NULL) {
#ifdef DEBUG_0
		printf("ERROR: Enqueuing to a NULL queue or inserting a NULL node.\n");
#endif /* ! DEBUG_0 */
        return RTX_ERR;
    }

    p_node->mp_next = NULL;
    if(p_queue->size == 0) {
        p_queue->first = p_node;
    } else {
        p_queue->last->mp_next = p_node;
    }

    p_queue->last = p_node;
		
		p_queue->size = p_queue->size + 1;
    return RTX_OK;
}

int msg_sorted_enqueue(k_msg_queue *p_queue, MSG_BUF *p_node) {
		MSG_BUF *previousNode = NULL;
		MSG_BUF *currentNode = p_queue->first;
	
		if(p_queue == NULL || p_node == NULL) {
#ifdef DEBUG_0
		printf("ERROR: Enqueuing to a NULL queue or inserting a NULL node.\n");
#endif /* ! DEBUG_0 */
        return RTX_ERR;
    }
		
		while(currentNode != NULL && p_node->m_kdata[0] >= currentNode->m_kdata[0]) {
				previousNode = currentNode;
				currentNode = currentNode->mp_next;
		}
		
		if(previousNode != NULL) {
				previousNode->mp_next = p_node;
		} else {
				p_queue->first = p_node;
		}
		
		if(currentNode != NULL) {
				p_node->mp_next = currentNode;
		} else {
				p_node->mp_next = NULL;
				p_queue->last = p_node;
		}
		
		p_queue->size = p_queue->size + 1;
    return RTX_OK;
}

MSG_BUF* msg_dequeue(k_msg_queue *p_queue) {
    MSG_BUF *p_first = NULL;

    if (p_queue->size == 0) {
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
	
		p_queue->size = p_queue->size - 1;
    return p_first;
}

int msg_q_is_empty(k_msg_queue *p_queue) {
    if(p_queue == NULL) {
#ifdef DEBUG_0
        printf("ERROR: Checking if a NULL queue is empty.\n");
#endif /* ! DEBUG_0 */
			return RTX_ERR;
    } else {
        return (p_queue->size == 0);
    }
}
