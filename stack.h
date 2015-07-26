/*
 * Simple stack data structure implementation.
 */

#ifndef _STACK_H
#define _STACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef list_t stack_t; /* Creates an alias for list. */

void stack_push(void *item, stack_t *stack);
void *stack_pop(stack_t *stack);
void *stack_peek(stack_t *stack);

stack_t *stack_alloc(size_t initial_capacity, size_t element_size);
void stack_free(stack_t *stack, void(*item_free_func)(void *));

#ifdef __cplusplus
}
#endif

#endif // _STACK_H