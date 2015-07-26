
#include "includes.h"


/* Converts a token to its respective string representation.
 * Returns an malloc'd buffer that must be freed with free() after it has been used.
 * Returns NULL if an error occurred.
 */
const char *tok_to_str(token_t t) {
	if (t.data_ptr != NULL) {
		int buffer_size = sizeof(char) * (t.data_len + 1);
		char *ret = (char *)malloc(buffer_size);

		if (ret == NULL) {
			errno = SLANG_MEM_ERR;
			raise_error("token.c", slang_create_error_token(16, 0), NULL);
			return NULL;
		}

		RETURN_VAL_IF_ERROR(STRNCPY(ret, buffer_size, t.data_ptr, t.data_len), NULL);

		// Add null-terminating character
		ret[t.data_len] = '\0';

		return ret;
	}
	else {
		errno = SLANG_INVALID_ARG;
		raise_error("token.c", t, NULL);
	}
}

/* Mainly for debugging purposes (to see the output of certain functions in an understandable format).
 * The function basically calls tok_to_str on every element of the list tok_list.
 *
 * The return value is malloc'd, so it is good to free it after it has been printed to the screen or something.
 */
const char *tok_list_to_str(list_t *tok_list) {
	int i;
	
	// First, loop through the tokens and sum up their lengths
	int ret_length = 0;
	for (i = 0; i < tok_list->length; i++) {
		ret_length += GET_TOKEN_AT(tok_list, i).data_len;
	}

	// Now, allocate the buffer
	int ret_buf_size = sizeof(char) * (ret_length + 1);		// We add 1 for the null terminating character
	char *ret_buf = (char *)malloc(ret_buf_size);

	// Loop through the tokens again, but this time strcating the string with ret_buf
	for (i = 0; i < tok_list->length; i++) {
		char *tok_str = tok_to_str(GET_TOKEN_AT(tok_list, i));
		if (i == 0) {
			// Strcpy instead
			if (strcpy_s(ret_buf, ret_buf_size, tok_str) != NOERROR) {
				perror("Error");
				return NULL;
			}
		}
		else {
			if (strcat_s(ret_buf, ret_buf_size, tok_str) != NOERROR) {
				perror("Error");
				return NULL;
			}
		}
		free(tok_str);
	}

	// Return
	return ret_buf;
}
