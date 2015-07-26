/*
 * Various error definitions for the Slang compiler.
 */

#ifndef _SLANG_ERR
#define _SLANG_ERR

#ifdef __cplusplus
extern "C" {
#endif

#define SLANG_NO_ERR		0	/* NOERROR */

/*
 * This tells the caller that the error message has most likely already been printed to the screen,
 * and that there is no need to call perror() in this case.
 *
 * TODO: Eventually replace all occurrences of ENOENT with GENERIC_ERROR for code clarity.
 */
#define SLANG_GENERIC_ERR		2	/* ENOENT */

// Probably just get rid of this:
#define SLANG_MEM_ERR			12	/* ENOMEM */

/* Begin Slang-specific codes */
#define SLANG_SYNTAX_ERR		141	/* errno.h uses integers up to 140, so we start Slang-specific errors at 141 */
#define SLANG_SEMANTIC_ERR		142	/* Error in semantics (incorrect usage of operators or types) */
#define SLANG_IO_ERR			143	/* File IO error. eg. file doesn't exist */
#define SLANG_COLLECTION_EMPTY	144
#define SLANG_PARSE_REJECTED	145
#define SLANG_MEM_ERR			146
#define SLANG_BAD_PARSE_TREE	147
#define SLANG_INVALID_ARG		148
#define SLANG_BAD_GRAMMAR		149
#define SLANG_UNEXPECTED_EOF	150

#define PRINTERROR slang_perror

#define RESET_ERRNO errno = NOERROR

typedef struct slang_error {
	int error_code;
	token_t error_token;	// The token where the error was caught
	const char *file_name;
	const char *extra_data;
} slang_error_t;

typedef struct slang_errno_state {
	queue_t *error_queue;
	bool has_error;
} slang_errno_state_t;


slang_errno_state_t *slang_create_errno_state();
void slang_free_errno_state(slang_errno_state_t *errno_state);

// This function releases the memory pointed to by error, so no need to free() it later.
void slang_queue_error(slang_error_t *error, slang_errno_state_t *errno_state);

slang_error_t *slang_create_error(const char *file_name, token_t error_token, const char *extra_data);

// Returns NULL if there is no error.
// TODO: Implement this function
slang_error_t *slang_get_most_recent_error(slang_errno_state_t *errno_state);

// Prints Slang-specific message for Slang-errors, and otherwise acts like normal perror and discards file_name and line_num.
// Pops the first error off the error stack, does nothing if the stack is empty.
void slang_perror(const char *str, slang_errno_state_t *errno_state);

// Prints the most recent error (the one at the bottom of the queue).
void slang_print_most_recent(const char *str, slang_errno_state_t *state);
token_t slang_create_error_token(int line_number, int col_number);

// Define a global state variable.
extern slang_errno_state_t *global_slang_errno_state;

// This macro acts like a function, so lowercase is OK here.
#define raise_error(file_name, token, extra_data) slang_queue_error(slang_create_error(file_name, token, extra_data), global_slang_errno_state)

#define CLEAR_ERROR_QUEUE RESET_ERRNO;\
	while (global_slang_errno_state->has_error == true) {\
		slang_perror(NULL, global_slang_errno_state);\
	}

#ifdef __cplusplus
}
#endif

#endif // _SLANG_ERR