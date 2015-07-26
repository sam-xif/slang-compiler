/*
 * Header that defines utilities for scanning input buffers and files into lists of tokens.
 */


#ifndef _SCANNER_H
#define _SCANNER_H


#ifdef __cplusplus
extern "C" {
#endif

extern char **keywords; // List of keywords of the language

// The list of possible states for the scanner to be in
typedef enum {
	SCAN_ERR = -1,
	SCAN_START = 0,
	SCAN_READING = 1,
	SCAN_FINISH = 2
} scanner_state;

// Scanner data structure
typedef struct {
	FILE *fp;				// If there is a file being scanned, this will be set.
	char *buf;				// Array of characters to be scanned. The file fp will be read into this buffer.
	unsigned int buf_length;	// Length of buf
	int index;				// Position within buf
	list_t *token_list;		// List of tokens that have been collected. NULL by default. It is set by the scan function
	scanner_state state;	// State of the scanner

	int line_number;		// The current line number of the input (used for reporting errors).
} scanner_t;


/* For use later on */
typedef scanner_t SL_scanner;
typedef scanner_state SL_scan_state;


// functions
void scan_file(FILE *fp, scanner_t *scan);
void scan_buffer(const char *buf, int length, scanner_t *scan);
token_t *scanner_get_token(const char *str, int *index, int length); /* Gets and returns the first token from a string str. 
											Also takes a pointer to an integer representing the current index in the string, 
											and the function increments index by the amount of characters that were read */

void scanner_reset(scanner_t *scan);
void scanner_free(scanner_t *scan, bool free_buf);
scanner_t *scanner_alloc();

int scanner_peek_char(int index, scanner_t *scan);

// Scanner utility functions
int scanner_find_next_char(char c, scanner_t *scan);
int scanner_find_next_char_from(char c, int index, scanner_t *scan);

#ifdef __cplusplus
}
#endif

#endif // _SCANNER_H