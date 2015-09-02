
#include "includes.h"

slang_errno_state_t *global_slang_errno_state;


slang_errno_state_t *slang_create_errno_state() {
	slang_errno_state_t *ret = MALLOC_ONE(slang_errno_state_t);
	ret->error_queue = queue_alloc(LIST_DEFAULT_CAPACITY, sizeof(slang_error_t));
	ret->has_error = false;
	return ret;
}


void slang_free_errno_state(slang_errno_state_t *errno_state) {
	queue_free(errno_state->error_queue, NULL);
	free(errno_state);
}


void slang_queue_error(slang_error_t *error, slang_errno_state_t *errno_state) {
	enqueue(error, errno_state->error_queue);
	errno_state->has_error = true;
	free(error);
}


slang_error_t *slang_create_error(const char *file_name, token_t error_token, const char *extra_data) {
	slang_error_t *ret = MALLOC_ONE(slang_error_t);
	if (file_name == NULL) ret->file_name = NULL;
	else { MALLOC_STRING(file_name, ret->file_name); }

	if (extra_data == NULL) ret->extra_data = NULL;
	else { MALLOC_STRING(extra_data, ret->extra_data); }

	ret->error_token = error_token;
	ret->error_code = errno;

	return ret;
}


token_t slang_create_error_token(int line_number, int col_number) {
	token_t *ret = MALLOC_ONE(token_t);
	ret->line_number = line_number;
	ret->col_number = col_number;
	return *ret;
}


void slang_perror(const char *str, slang_errno_state_t *errno_state) {
	// 141 is the start of slang error codes, so anything less than that is assumed
	// to be defined in errno.h
	slang_error_t *error = NULL;


	if (errno_state->error_queue->length > 0) {
		error = dequeue(errno_state->error_queue);
		if (errno_state->error_queue->length == 0) {
			errno_state->has_error = true;
		}
	}
	else {
		errno_state->has_error = false;
		return;
	}

	errno_t save_errno = errno;
	errno = error->error_code;

	if (errno < 141) {
		perror(str);
	}
	else {
		if (error->file_name != NULL) {
			printf(error->file_name);
			printf(":");
		}
		printf("line %i, col %i:", error->error_token.line_number, error->error_token.col_number);
		if (str != NULL) {
			printf(str);
			printf(": ");
		}
		else printf(" ");
		
		switch (errno) { 
		case SLANG_GENERIC_ERROR:
			printf("An error occurred");
			break;
		case SLANG_SYNTAX_ERROR:
			printf("Syntax error");
			break;
		case SLANG_SEMANTIC_ERR:
			printf("Semantic error");
			break;
		case SLANG_IO_ERROR:
			printf("Error during file operation");
			break;
		case SLANG_COLLECTION_EMPTY:
			printf("The collection of items is empty");
			break;
		case SLANG_PARSE_REJECTED:
			printf("The parse was rejected, possibly due to a syntax error");
			break;
		case SLANG_MEM_ERROR:
			printf("Error while attempting to allocate memory");
			break;
		case SLANG_INVALID_ARG:
			printf("The argument passed was invalid");
			break;
		case SLANG_BAD_GRAMMAR:
			printf("The grammar that was found is invalid");
			break;
		case SLANG_UNEXPECTED_EOF:
			printf("Unexpected EOF while reading stream");
			break;
		}
	}

	if (error->extra_data != NULL) {
		printf(error->extra_data);
	}

	// Add newline.
	printf("\n");

	free(error);
	errno = save_errno;
}