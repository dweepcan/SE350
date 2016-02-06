#ifndef K_STACK_H
#define K_STACK_H

#include "k_rtx.h"
#include "k_node.h"

typedef struct k_stack {
	k_node *top;
} k_stack;

void push(k_stack *p_stack, k_node *p_node);
k_node* pop(k_stack *p_stack);
int contains(k_stack *p_stack, k_node *p_node);
int is_empty(k_stack *p_stack);

#endif //K_STACK_H
