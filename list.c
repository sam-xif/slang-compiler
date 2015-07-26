
#include "includes.h"

// Allocate and initialize a new arraylist
list_t *list_alloc(size_t initial_capacity, size_t element_size) {
	list_t *list = MALLOC_ONE(list_t);
	if (list == NULL) {
		puts("Error allocating memory with an malloc call.");
		errno = ENOMEM;
		return NULL;
	}
	list->length = 0;
	list->immutable = false;
	list->capacity = initial_capacity;
	list->element_size = element_size;
	list->array = malloc(list->capacity * list->element_size);
	return list;
}

/*
 * Function usage:
 *
 * This function frees the list from memory, but it allows the user to specify a function
 * that will be called for every element in the list before the list itself is freed.
 * This allows for each item itself to be freed if it was malloc'd, thereby preventing memory leaks.
 * NULL can also be passed for item_free_func, which turns off this behavior.
 *
 * The function passed can also ONLY free items from within the structure. For example,
 *
 * * There is a structure which contains a char *str;
 * * This str is malloc'd when a new instance of the struct is created.
 *
 * When an instance is added to the list, the struct's data is memcpy'd to the list's allocated memory block,
 * and this now removes the need to free() each element of the list individually, since the data was completely deep-copied over,
 * and exists in the list's allocated memory.
 *
 * Now, str is still a pointer to a string (deep copying the struct didn't change or copy the value that str pointed to), 
 * and it must be free()'d. This can be accomplished through the item_free_func pointer.
 *
 * The recommended way to create an item_free_func is to create a static function that frees all the pointers in each instance,
 * and nothing more. Other logic can be performed in this function if needed, but that is not what this was intended for.
 */
void list_free(list_t *list, void(*item_free_func)(void *)) {
	if (list == NULL) return; // Do nothing. Same behavior as free()

	if (item_free_func != NULL) {
		int i;
		for (i = 0; i < list->length; i++) {
			item_free_func(GET_ITEM_AT(list, i));
		}
	}

	free(list->array);
	free(list);
}

void list_free_no_func(list_t *list) {
	list_free(list, NULL);
}


/* ====== LIST MANIPULATION FUNCTIONS ====== */


/* 
 * Adds an item to an existing list.
 * Sets errno and prints a message to the console if an error occurrs.
 */
void list_add_item(void *item, list_t *list) {
	assert(sizeof(char) == 1);

	assert(item != NULL);
	assert(list != NULL);
	assert(list->array != NULL);


	// No real point in having these if statements here because there are asserts right above...
	// Maybe add messages to these two as well.
	if (item == NULL) { errno = EINVAL; return; }
	if (list == NULL) { errno = EINVAL; return; }

	if (list->array == NULL) {
		puts("A list that has been allocated with list_alloc() must be passed to add_item().");
		errno = EINVAL;
		return;
	}

	if (list->immutable == true) {
		// Error
		puts("This list is immutable. It's contents can no longer be modified.");
		errno = EINVAL;
		return;
	}

	if (memcpy_s((void *)((char *)list->array + (list->length * list->element_size)),
		list->element_size, item, list->element_size) != NOERROR) {
		perror("Error");
		return;
	}
	list->length++;
	if (list->length == list->capacity) {
		list->capacity = list->capacity * 2;

		int new_size = list->capacity * list->element_size;
		void *tmp_buf = malloc(new_size / 2);

		if (tmp_buf == NULL) {
			puts("Error allocating memory with an malloc call.");
			errno = ENOMEM;
			return;
		}

		if (memcpy_s(tmp_buf, new_size / 2, list->array, new_size / 2) != NOERROR) {
			perror("Error");
			free(tmp_buf);
			return;
		}

		list->array = realloc(list->array, new_size);

		if (list->array == NULL) {
			puts("Error when resizing the array.");
			errno = ENOMEM;
			free(tmp_buf);
			return;
		}

		if (memcpy_s(list->array, new_size, tmp_buf, new_size / 2) != NOERROR) {
			perror("Error");
			free(tmp_buf);
			return;
		}

		free(tmp_buf); // All done
	}
}

void list_insert_item(void *item, int index, list_t *list) {
	assert(sizeof(char) == 1);

	assert(item != NULL);
	assert(list != NULL);
	assert(list->array != NULL);

	// No real point in having these if statements here...
	// Maybe add messages to these two as well.
	if (item == NULL) { errno = EINVAL; return; }
	if (list == NULL) { errno = EINVAL; return; }

	if (list->array == NULL) {
		puts("A list that has been allocated with list_alloc() must be passed to add_item().");
		errno = EINVAL;
		return;
	}

	if (list->immutable == true) {
		// Error
		puts("This list is immutable. It's contents can no longer be modified.");
		errno = EINVAL;
		return;
	}

	int bytes_to_move = (list->length - index) * list->element_size;
	if (memmove_s(GET_ITEM_AT(list, index + 1), bytes_to_move, GET_ITEM_AT(list, index), bytes_to_move) != NOERROR) {
		perror("Error");
		return;
	}
	if (memcpy_s(GET_ITEM_AT(list, index), list->element_size, item, list->element_size) != NOERROR) {
		perror("Error");
		return;
	}
	list->length++;
	if (list->length == list->capacity) {
		list->capacity = list->capacity * 2;

		int new_size = list->capacity * list->element_size;
		void *tmp_buf = malloc(new_size / 2);

		if (tmp_buf == NULL) {
			puts("Error allocating memory with an malloc call.");
			errno = ENOMEM;
			return;
		}

		if (memcpy_s(tmp_buf, new_size / 2, list->array, new_size / 2) != NOERROR) {
			perror("Error");
			free(tmp_buf);
			return;
		}

		list->array = realloc(list->array, new_size);

		if (list->array == NULL) {
			puts("Error when resizing the array.");
			errno = ENOMEM;
			free(tmp_buf);
			return;
		}

		if (memcpy_s(list->array, new_size, tmp_buf, new_size / 2) != NOERROR) {
			perror("Error");
			free(tmp_buf);
			return;
		}

		free(tmp_buf); // All done
	}
}


void list_remove_item(int index, list_t *list) {
	assert(sizeof(char) == 1);

	assert(list != NULL);
	assert(list->array != NULL);

	// No real point in having these if statements here...
	// Maybe add messages to these two as well.
	if (list == NULL) { errno = EINVAL; return; }

	if (list->length > 0) {
		// Relying on the fact that one element is being removed every time.
		int bytes_to_move = (list->length - (index + 1)) * list->element_size;
		RETURN_IF_ERROR(MEMMOVE(GET_ITEM_AT(list, index), bytes_to_move + list->element_size,
			GET_ITEM_AT(list, index + 1), bytes_to_move));
		list->length--;
	}
	else {
		errno = SLANG_COLLECTION_EMPTY;
	}
}

void *list_get_item(int index, list_t *list) {
	void *ret = malloc(list->element_size);
	if (ret == NULL) {
		puts("Error allocating memory with an malloc call");
		errno = ENOMEM;
		return NULL;
	}

	/*
	 ************************************************************************
	 *
	 * TODO: Update all the functions in this file to use macros like this
	 *
	 ************************************************************************
	 */
	RETURN_VAL_IF_ERROR(MEMCPY(ret, list->element_size, 
		GET_ITEM_AT(list, index), list->element_size), NULL);

	return ret;
}

// Removes extra allocated memory from the list
void list_trim(list_t *list) {
	assert(list != NULL);

	// No real point in having these if statements here...
	if (list == NULL) { errno = EINVAL; return; }

	if (list->immutable == true){
		puts("This list has already been trimmed.");
		errno = EINVAL; // Error hasn't necessarily occurred, but we have to tell the caller that the list was never trimmed.
		return;
	}

	int new_size = list->length * list->element_size; // just a handy variable to save me from typing
	void *tmp_buf = malloc(new_size);

	if (tmp_buf == NULL) {
		puts("Error when allocating memory with an malloc call.");
		errno = ENOMEM;
		return;
	}

	// Don't use macros here because we have to free tmp_buf
	if (memcpy_s(tmp_buf, new_size, list->array, new_size) != NOERROR) {
		perror("Error");
		free(tmp_buf);
		return;
	}

	list->array = realloc(list->array, new_size);

	if (list->array == NULL) {
		puts("Error when resizing the array.");
		errno = ENOMEM;
		free(tmp_buf);
		return;
	}

	if (memcpy_s(list->array, new_size, tmp_buf, new_size) != NOERROR) {
		perror("Error");
		free(tmp_buf);
		return;
	}

	list->capacity = list->length;

	free(tmp_buf);
}

// Returns dest or NULL if error
void *list_copyto(list_t *list_src, void *dest, int destSize) {
	
	// Check for null values:
	assert(list_src != NULL);
	assert(dest != NULL);

	// Make sure the buffer is big enough to hold everything
	assert(destSize >= (list_src->length * list_src->element_size));


	// Use the new macros here and in other functions:

	// Copy the data, checking for errors
	RETURN_VAL_IF_ERROR(MEMCPY(dest, destSize, list_src->array, list_src->length * list_src->element_size), NULL);

	return dest;
}


bool list_contains_item(void *item, list_t *list, bool(*equality_compare_func)(void *, void *)) {
	assert(equality_compare_func != NULL);
	assert(item != NULL);
	assert(list != NULL);

	// Dont bother with the if statements that are just duplicates of the asserts
	

	int i;
	for (i = 0; i < list->length; i++) {
		if (equality_compare_func(GET_ITEM_AT(list, i), item) == true)
			return true;
	}

	return false;
}

list_t *list_concatenate(list_t *list1, list_t *list2) {
	// Basic assertions
	assert(list1 != NULL);
	assert(list2 != NULL);
	
	int i;
	for (i = 0; i < list2->length; i++) {
		list_add_item(GET_ITEM_AT(list2, i), list1);
	}

	return list1;
}