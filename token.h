/*
 * Header that provides utilities for dealing with tokens and lists of tokens
 */

#ifndef _TOKEN_H
#define _TOKEN_H

#include "includes.h"

// TODO: Fix get_token so that it recognizes this as a line comment
#define COMMENT_CHAR ('#')

#define TOKEN_LIST_DEFAULT_CAPACITY 30

/* A macro to simplify casting the list_t to a token*
 * Allows you to get a token at index i in the specified list.
 * 
 * list is of type list_t*. i is of type integer.
 */
#define GET_TOKEN_AT(list, i) (((token_t *)(list->array))[i])

// A string consisting of all the characters that are defined as whitespace
#define WHITESPACE ("\n\t\r ")

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The token_type enum.
 * TODO: Rename this to token_kind because _type is a bit odd (especially for _t suffix).
 *	Also rename other enums to _kind instead of _type.
 */
typedef enum token_type {
	
	// Make all of these names all caps one day...

	// Tokens that will always be one character:
	token_newline = '\n',
	token_cr = '\r',
	token_tab = '\t',
	token_space = ' ',
	token_minus = '-',
	token_plus = '+',
	token_mult = '*',
	token_div = '/',
	token_equal = '=',
	token_dot = '.',
	token_comma = ',',
	token_greaterthan = '>',
	token_lessthan = '<',
	token_openparen = '(',
	token_closeparen = ')',
	token_semicolon = ';',
	token_colon = ':',
	token_caret = '^',
	token_open_curly = '{',
	token_close_curly = '}',
	token_open_brack = '[',
	token_close_brack = ']',
	token_single_quote = ',',
	token_dollar_sign = '$',
	token_exclamation = '!',
	token_hash = '#',
	token_underscore = '_',

	// Tokens that may be more than one character, as well as language specific tokens:
	token_id = 131,			// variable and function names (things that "identify")
	token_string = 133,
	token_leftarrow = 134,
	token_double_eq = 135,
	token_not_eq = 136,
	token_greater_eq = 137,
	token_less_eq = 138,
	token_integer = 150,
	token_decimal = 151,
	token_whitespace = 152,
	token_epsilon = 153,

	// Represents the end of the token stream. Used mainly by the parser
	token_end_of_stream = -2,

	// Error/undefined token
	token_undefined = -1

	// add more as needed ...
} token_kind; /* Defines all generic and language-specific tokens for use with the scanner */

typedef struct token {
	token_kind type;
	const char *data_ptr;		/* Only used by some token types. One- or two- character tokens that are consistent in their data
						   do not really need this, so in those cases, it will be NULL */
	unsigned int data_len;		// For strings, names, numbers, etc. This is the length of the token's data. If data_ptr is NULL, this is 0
	int index_in_buf;	// The index of the token within the buffer.
	int line_number;	// The line number that this token is found on in the input. Used for error printing.
	int col_number;
} token_t; /* Represents a single token */

/* For use later on */
typedef token_t slang_token_t;
typedef token_kind slang_token_type;

typedef list_t token_list_t;


const char *tok_to_str(token_t t);
const char *tok_list_to_str(list_t *tok_list);


#ifdef __cplusplus
}
#endif

#endif // _TOKEN_H