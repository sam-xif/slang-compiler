/*
 * A header that defines utilities for creating arraylists of one type, 
 * that automatically allocate more memory to the array as more items are added.
 */


#ifndef _LIST_H
#define _LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#define LIST_DEFAULT_CAPACITY 16
#define GET_ITEM_AT(list, i) ((void *)((char *)list->array + ((i) * list->element_size)))
#define GET_ITEM_AS(list, i, t) ((t *)((char *)list->array + ((i) * list->element_size)))

/* 
 * This was designed for use with only one type of object, 
 * but there are probably ways to store objects that have a size
 * smaller than element_size...
 */
typedef struct list_t {
	size_t element_size;	// This must be set for add_item to work correctly
	size_t capacity;		// The maximum number of elements the array can have before it has to reallocate. Avoid manually changing this value.
	int length;				// The number of elements currently in the array
	bool immutable;			// False by default. After the list is trimmed, the list becomes immutable, and this is set to true.
	void *array;			// The actual array. The pointer has to be casted to the desired type to be dereferenced.
} list_t;					// Arraylist implementation

/* For use later on */
typedef list_t SL_arraylist;

list_t *list_alloc(size_t initial_capacity, size_t element_size);

#define LIST_CREATE(t) list_alloc(LIST_DEFAULT_CAPACITY, sizeof(t))

void list_free(list_t *list, void(*item_free_func)(void *));

// These functions are needed to release multi-dimensional list structures from memory.
void list_free_no_func(list_t *list);
void list_free_array_only(list_t *list);

void list_add_item(void *item, list_t *list);
void list_insert_item(void *item, int index, list_t *list);

// Removes an item from a list, and should return the item that was removed, so that pointers can be freed.
void list_remove_item(int index, list_t *list);

void *list_get_item(int index, list_t *list);

// Removes extra allocated memory from the list
void list_trim(list_t *list);

// TODO: Add a way to extract an array pointer from the list while also being able to free the list. (memcpy)
// Copy array to location. The location must be big enough to hold the entire array, or an assertion error will be thrown.
// Returns: dest
void *list_copyto(list_t *list_src, void *dest, int destSize);

// Appends list2's elements to the end of list1. Returns list1.
// Maybe switch the parameter order here.
list_t *list_concatenate(list_t *list1, list_t *list2);

bool list_contains_item(void *item, list_t *list, bool(*equality_compare_func)(void *, void *));

#ifdef __cplusplus
}
#endif

#endif // _LIST_H