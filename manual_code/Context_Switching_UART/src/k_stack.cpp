#include "k_stack.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

int push(k_stack *p_stack, k_node *p_node) {
	if(p_stack == NULL || p_node == NULL) {
#ifdef DEBUG_0
		printf("ERROR: Pushing to a NULL stack or inserting a NULL node.");
#endif /* ! DEBUG_0 */
		return RTX_ERR;
	}

	p_node->next = p_stack->top;
	p_stack->top = p_node;

	return RTX_OK;
}

k_node *pop(k_stack *p_stack) {
	k_node *p_first = NULL;

	if(is_empty(p_stack)) {
#ifdef DEBUG_0
		printf("ERROR: Popping a node from an empty stack.");
#endif /* ! DEBUG_0 */
		return NULL;
	}

	p_first = p_stack->top;
	p_stack->top = p_stack->top->next;

	return p_first;
}

int contains(k_stack *p_stack, k_node *p_node) {
	k_node *p_curr = NULL;

	if(is_empty(p_stack)) {
#ifdef DEBUG_0
		printf("ERROR: Checking if a node exists in a empty stack.\n");
#endif /* ! DEBUG_0 */
		return 0;
	}

	p_curr = p_stack->top;
	while(p_curr != NULL) {
		if(p_curr == p_node) {
			return 1;
		}
		p_curr = p_curr->next;
	}

	return 0;
}

int is_empty(k_stack *p_stack) {
	if(p_stack == NULL) {
#ifdef DEBUG_0
		printf("ERROR: Checking if a NULL stack is empty.");
#endif /* ! DEBUG_0 */
		return 0;
	} else {
		return (p_stack->top == NULL);
	}
}
