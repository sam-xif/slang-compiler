
#include "includes.h"

void stack_push(void *item, stack_t *stack) {
	list_add_item(item, (list_t *)stack);
}

// Returns NULL and sets errno = STACK_EMPTY_ERR if there are no elements left to be popped.
void *stack_pop(stack_t *stack) {
	// Possibly paste code from stack_peek to speed up this function if I see fit...
	void *peek = stack_peek(stack);
	void *ret = malloc(sizeof(stack->element_size));
	if (ret != NULL) {
		RETURN_VAL_IF_ERROR(MEMCPY(ret, stack->element_size, peek, stack->element_size), NULL);
	}
	else {
		errno = SLANG_MEM_ERROR;
		raise_error("queue.c", slang_create_error_token(16, 0), NULL);
	}

	if (errno == SLANG_COLLECTION_EMPTY) return NULL;
	list_remove_item(stack->length - 1, stack);
	return ret;
}

void *stack_peek(stack_t *stack) {
	if (stack->length == 0) {
		errno = SLANG_COLLECTION_EMPTY;
		return NULL;
	}
	return GET_ITEM_AT(((list_t *)stack), stack->length - 1);
}

stack_t *stack_alloc(size_t initial_capacity, size_t element_size) {
	return (stack_t *)list_alloc(initial_capacity, element_size);
}

void stack_free(stack_t *stack, void (*item_free_func)(void *)) {
	list_free(stack, item_free_func);
}