/*
* Simple queue data structure implementation.
*/

#ifndef _QUEUE_H
#define _QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef list_t queue_t; /* Creates an alias for list. */

void enqueue(void *item, queue_t *queue);
void *dequeue(queue_t *queue);
void *queue_peek(queue_t *queue);

queue_t *queue_alloc(size_t initial_capacity, size_t element_size);
void queue_free(stack_t *queue, void(*item_free_func)(void *));

#ifdef __cplusplus
}
#endif

#endif // _QUEUE_H