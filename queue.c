
#include "includes.h"

void enqueue(void *item, queue_t *queue) {
	list_add_item(item, queue);
}

// Returns an malloc'd pointer to the first item of the queue, which was removed in this function.
// It must be deallocated after use with free().
void *dequeue(queue_t *queue) {
	void *peek = queue_peek(queue);
	void *ret = malloc(queue->element_size);
	if (ret != NULL) {
		RETURN_VAL_IF_ERROR(MEMCPY(ret, queue->element_size, peek, queue->element_size), NULL);
	}
	else {
		errno = SLANG_MEM_ERROR;
		raise_error("queue.c", slang_create_error_token(16, 0), NULL);
		return NULL;
	}
	
	if (errno == SLANG_COLLECTION_EMPTY) return NULL;
	
	list_remove_item(0, queue);
	return ret;
}

// Returns a pointer to the first item in the queue.
void *queue_peek(queue_t *queue) {
	if (queue->length == 0) {
		errno = SLANG_COLLECTION_EMPTY;
		return NULL;
	}
	return GET_ITEM_AT(queue, 0);
}

queue_t *queue_alloc(size_t initial_capacity, size_t element_size) {
	return (queue_t *)list_alloc(initial_capacity, element_size);
}

void queue_free(stack_t *queue, void(*item_free_func)(void *)) {
	list_free(queue, item_free_func);
}
