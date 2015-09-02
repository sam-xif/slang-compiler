
#include "includes.h"

// Scans a file opened in "rb" mode
// TODO: Make the return type a bool possibly. (For code consistency)
void scan_file(const char *file_name, scanner_t *scan) {
	scan->file_name = file_name;
	scan->scanning_file = true;
	FILE *fp;

	if (file_name == NULL) {
		errno = SLANG_INVALID_ARG;
		raise_error("scanner.c", slang_create_error_token(11, 0), "fp cannot be NULL");
		return;
	}
	if (scan == NULL) {
		errno = SLANG_INVALID_ARG;
		raise_error("scanner.c", slang_create_error_token(11, 0), "scan cannot be NULL");
		return;
	}
	if (fopen_s(&fp, file_name, "rb") != NOERROR) {
		errno = SLANG_IO_ERROR;
		raise_error("scanner.c", slang_create_error_token(24, 0), "Error occurred while opening file");
		return;
	}

	scan->token_list = list_alloc(TOKEN_LIST_DEFAULT_CAPACITY, sizeof(token_t));

	if (fseek(fp, 0, SEEK_END) != 0) {
		if (ferror(fp)) {
			errno = SLANG_IO_ERROR;
			raise_error("scanner.c", slang_create_error_token(20, 0), "Seek operation failed");
		}
		scan->state = SCAN_ERR;
		return;
	}
	if ((scan->buf_length = (int)ftell(fp)) == -1) {
		scan->state = SCAN_ERR;
		errno = SLANG_IO_ERROR;
		raise_error("scanner.c", slang_create_error_token(25, 0), "ftell failed");
		return;
	}
	if (fseek(fp, 0, SEEK_SET) != 0) {
		if (ferror(fp)) puts("Error while seeking in the file");
		scan->state = SCAN_ERR;
		return;
	}
	if ((scan->buf = (char *)malloc((sizeof(char) * scan->buf_length) + 1)) == NULL) {
		scan->state = SCAN_ERR;
		errno = SLANG_MEM_ERROR;
		raise_error("scanner.c", slang_create_error_token(36, 0), NULL);
		return;
	}
	if (fread(scan->buf, sizeof(char), scan->buf_length, fp) == 0) {
		if (ferror(fp)) {
			errno = SLANG_IO_ERROR;
			printf("scanner.c:39: Error occurred while reading file");
		}
		if (feof(fp)) puts(":\nEOF was reached"); // Appends a newline char
		else if (errno != NOERROR) slang_perror("Error", "scanner.c", 43);
		else printf("\n");
		scan->state = SCAN_ERR;
		return;
	}

	// Cast away the const to add a null-terminator
	((char *)scan->buf)[scan->buf_length] = '\0'; 

	int last_newline_index = 0;

	scan->state = SCAN_READING;
	while (scan->index < scan->buf_length) {
		token_t *t = scanner_get_token(scan->buf, &scan->index, scan->buf_length);
		if (t != NULL) {
			if (t->type == token_newline) {
				last_newline_index = t->index_in_buf + 1;
				scan->line_number++;
			}
			t->line_number = scan->line_number;
			t->col_number = t->index_in_buf - last_newline_index;
			list_add_item(t, scan->token_list);
		}
		else {
			//puts("Unexpected EOF or other error while scanning tokens");
			scan->state = SCAN_ERR;
			errno = SLANG_UNEXPECTED_EOF;
			raise_error("scanner.c", GET_TOKEN_AT(scan->token_list, scan->token_list->length - 1), NULL);
			return;
		}
		free(t);
	}

	// Add an end_of_stream token
	token_t eos;
	eos.type = token_end_of_stream;
	eos.data_ptr = scan->buf + scan->buf_length;
	eos.data_len = 1;
	eos.index_in_buf = scan->buf_length;

	list_add_item(&eos, scan->token_list);

	scan->state = SCAN_FINISH;
}


void scan_buffer(const char *buf, int length, scanner_t *scan) {
	// Assume that scan->buf != buf

	scan->buf = buf;
	scan->buf_length = length;
	scan->token_list = list_alloc(TOKEN_LIST_DEFAULT_CAPACITY, sizeof(token_t));

	int last_newline_index = 0;

	while (scan->index < scan->buf_length) {
		token_t *t = scanner_get_token(scan->buf, &scan->index, scan->buf_length);
		if (t != NULL) {
			if (t->type == token_newline) {
				last_newline_index = t->index_in_buf + 1;
				scan->line_number++;
			}
			t->line_number = scan->line_number;
			t->col_number = t->index_in_buf - last_newline_index;
			list_add_item(t, scan->token_list);
		}
		else {
			//puts("Unexpected EOF or other error while scanning tokens");
			scan->state = SCAN_ERR;
			errno = SLANG_UNEXPECTED_EOF;
			raise_error("scanner.c", GET_TOKEN_AT(scan->token_list, scan->token_list->length - 1), NULL);
			return;
		}
		free(t);
	}

	// Add an end_of_stream token
	token_t eos;
	eos.type = token_end_of_stream;
	eos.data_ptr = buf + length;
	eos.data_len = 1;
	eos.index_in_buf = length;

	list_add_item(&eos, scan->token_list);

	scan->state = SCAN_FINISH;
}

// get_token is probably a pretty badly written function lol

/* 
 * Gets the first token to be found after index in the string pointed to by str.
 * The variable index is incremented by the amount of characters read. 
 * The return value is malloc'd, so it is good practice to free() the return value after it has been used,
 * or free it immediately if it will not be used at all.
 */
token_t *scanner_get_token(const char *str, int *index, int length) {
	if (*index >= length) {
		errno = SLANG_UNEXPECTED_EOF;
		raise_error("scanner.c", slang_create_error_token(144, 0), NULL);
		return NULL;
	}
	else {
		// define some temporary variables
		int c = 0;
		int tmp = 0;
		int init_index = *index; // Save the original value (could be of use later for debugging)
		//int i = *index;
		token_t t;
		t.data_len = 0;
		t.data_ptr = NULL;
		t.index_in_buf = 0;
		t.line_number = 0;
		t.col_number = 0;
		t.type = token_undefined;

		// Build a temporary scanner object with the information we have
		scanner_t s;
		s.buf = str;
		s.index = *index;
		s.buf_length = length;
		s.state = SCAN_READING;

		switch ((c = str[*index])) {
		case '\'':
			tmp = *index + 1;

			do {
				tmp = scanner_find_next_char_from('\'', tmp, &s);
				if (tmp == -1) {
					puts("Unexpected EOF or other error occurred");
					return NULL;
				}
			} while (scanner_peek_char(tmp - 1, &s) == ESCAPE_CHAR);

			goto finish_string;		// To avoid double initialization and duplicate code.
			break;

		case '"':
			tmp = *index + 1;

			do {
				tmp = scanner_find_next_char_from('"', tmp, &s);
				if (tmp == -1) {
					puts("Unexpected EOF or other error occurred");
					return NULL;
				}
			} while (scanner_peek_char(tmp - 1, &s) == ESCAPE_CHAR);

		finish_string:

			if (tmp > length) {
				errno = SLANG_UNEXPECTED_EOF;
				raise_error("scanner.c", slang_create_error_token(144, 0), NULL);
				return NULL;
			}

			/* Now next_quote is the end of the string. We increment the index to pass the quotation mark at the beginnning of
			the string */
			int str_length = tmp - (++(*index));

			t.type = token_string;

			// TODO: These three lines are repeated throughout this function, so remove
			// the redundancies and put them after the switch statement or something.
			t.data_ptr = str + *index;
			t.data_len = str_length;
			*index = *index + str_length;
			break;

		case '<':
			if (*index + 1 == length) break;
			if (scanner_peek_char(*index + 1, &s) == '=') {
				t.type = token_less_eq;
				t.data_ptr = str + *index;
				t.data_len = 2;
				
				// Add one to the two-char tokens.
				++(*index);
			}
			else {
				t.type = token_lessthan;
				t.data_ptr = str + *index;
				t.data_len = 1;
			}
			break;

		case '>':
			if (*index + 1 == length) break;
			if (scanner_peek_char(*index + 1, &s) == '=') {
				t.type = token_greater_eq;
				t.data_ptr = str + *index;
				t.data_len = 2; 
				++(*index);
			}
			else {
				t.type = token_greaterthan;
				t.data_ptr = str + *index;
				t.data_len = 1;
			}
			break;

		case '=':
			if (*index + 1 == length) break;
			if (scanner_peek_char(*index + 1, &s) == '=') {
				t.type = token_double_eq;
				t.data_ptr = str + *index;
				t.data_len = 2;
				++(*index);
			}
			else {
				t.type = token_equal;
				t.data_ptr = str + *index;
				t.data_len = 1;
			}
			break;

		case '!':
			if (*index + 1 == length) break;
			if (scanner_peek_char(*index + 1, &s) == '=') {
				t.type = token_not_eq;
				t.data_ptr = str + *index;
				t.data_len = 2;
				++(*index);
			}
			else {
				t.type = token_exclamation;
				t.data_ptr = str + *index;
				t.data_len = 1;
			}
			break;


		default_case:
		default:
			if (isalpha(c) || c == '_') {
				tmp = *index;
				while ((isalpha(c) || isdigit(c) || c == '_') && (tmp < length)) {
					tmp++;

					if (tmp < length) c = str[tmp]; else break;
				}

				// Now, scan->index represents the first character of the name
				// Identify the type of word (i.e. keyword, id, ...)
				t.type = token_id;
				t.data_ptr = str + *index;
				t.data_len = tmp - *index;

				*index = *index + t.data_len - 1;
			}
			else if (isdigit(c) || c == '-') {
				t.type = token_integer;
				tmp = *index;
				bool is_negative = false;
				if (c == '-') {
					is_negative = true;
					tmp++;
					if (tmp < length) c = str[tmp];
					else {
						errno = SLANG_UNEXPECTED_EOF;
						raise_error("unknown", slang_create_error_token(0, 0), NULL);
					}
				}
				while (isdigit(c)) {
					tmp++;
					if (scanner_peek_char(tmp, &s) == DECIMAL_SEPARATOR) {
						if (t.type != token_decimal) {
							t.type = token_decimal;
							tmp++;
						}
						else {
							errno = SLANG_SYNTAX_ERROR;
							raise_error("scanner.c", slang_create_error_token(318, 0), "Too many decimal points in number");
							return NULL;
						}
					}

					if (tmp < length) c = str[tmp]; else break;
				}

				// matched only the '-'
				if (is_negative == true && tmp == (*index) + 1)
					goto add_single_character;

				t.data_ptr = str + *index;
				t.data_len = tmp - *index;

				*index = *index + t.data_len - 1;
			}
			else {
			add_single_character:

				// Last resort: Verify that the character is an ASCII character and hope that it's part of the token enum:
				if (c < 128 && c >= 0) {
					t.type = (token_kind)c;
					t.data_len = 1;
					t.data_ptr = str + *index;
				}
				else {
					errno = SLANG_SYNTAX_ERROR;
					raise_error("scanner.c", slang_create_error_token(347, 0), "The token cannot be matched");
					return NULL;
				}
			}
			break;
		}


		// Epilogue

		(*index)++;

		// Set the index of the first character of the token in the buffer
		t.index_in_buf = t.data_ptr - str;

		token_t *ret = (token_t *)malloc(sizeof(token_t));
		*ret = t;

		return ret;
	}
}

void scanner_reset(scanner_t *scan) {
	if (scan->fp != NULL) {
		if (fclose(scan->fp) == EOF) {
			puts("Error closing file when freeing scanner data memory");
			errno = ENOENT;
			return;
		}
	}

	free(scan->buf);
	list_free(scan->token_list, NULL);
	scan->buf = NULL;
	scan->buf_length = 0;
	scan->token_list = NULL;
	scan->index = 0;
	scan->state = SCAN_START;
	scan->line_number = 1;
	if (errno != NOERROR) {
		perror("Error");
		return;
	}
}

void scanner_free(scanner_t *scan, bool free_buf) {
	if (scan == NULL) return;

	if (free_buf)
		free(scan->buf);

	list_free(scan->token_list, NULL);
	if (scan->fp != NULL) {
		if (fclose(scan->fp) == EOF) {
			puts("Error closing file when freeing scanner data memory");
			//
			return;
		}
	}
	free(scan);
	if (errno != NOERROR) {
		raise_error("scanner.c", slang_create_error_token(399, 0), NULL);
		return;
	}

}

// Allocates a new scanner object and sets all of its values to 0
scanner_t *scanner_alloc() {
	scanner_t *scan = MALLOC_ONE(scanner_t);
	if (scan == NULL) {
		errno = SLANG_MEM_ERROR;
		raise_error("scanner.c", slang_create_error_token(417, 0), NULL);
		return NULL;
	}
	scan->fp = NULL;
	scan->buf = NULL;
	scan->buf_length = 0;
	scan->token_list = NULL;
	scan->index = 0;
	scan->state = SCAN_START;
	scan->line_number = 1;
	return scan;
}

int scanner_peek_char(int index, scanner_t *scan) {
	if (scan->buf != NULL && index < scan->buf_length)
		return scan->buf[index];
	else
		return -1;
}

/*
 * The next two functions, find_next and find_next_from, search for the next character c to be found in a buffer.
 * The buffer is contained within the scanner object, and in find_next, the index value and length are also taken from the object.
 * In find_next_from, however, only the buffer length is taken, and the index is a user passed value. This is useful for when you
 * want to skip ahead or go back a few characters before searching.
 */

// Uses the index from the scan object
int scanner_find_next_char(char c, scanner_t *scan) {
	int index;
	
	if (scan == NULL) { errno = SLANG_INVALID_ARG; return -1; }
	if (scan->buf == NULL) { errno = SLANG_INVALID_ARG; return -1; }
	if (scan->buf_length == 0) { errno = SLANG_INVALID_ARG; return -1; }

	for (index = scan->index; index < scan->buf_length; index++) {
		if (scan->buf[index] == c) break; 
	}
	return index;
}

// User passes the index value
int scanner_find_next_char_from(char c, int index, scanner_t *scan) {
	int _index;
	
	if (scan == NULL) { errno = SLANG_INVALID_ARG; return -1; }
	if (scan->buf == NULL) { errno = SLANG_INVALID_ARG; return -1; }
	if (scan->buf_length <= 0) { errno = SLANG_INVALID_ARG; return -1; }

	for (_index = index; _index < scan->buf_length; _index++) {
		if (scan->buf[_index] == c) break;
	}
	return _index;
}
