/*
 * Defines various utility functions used by the scanner and other parts of the compiler.
 */

#ifndef _LEXER_UTIL_H
#define _LEXER_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

// Possibly move these functions without the module prefix somewhere else
int find_next_char_from(char c, int index, char *buf, int buf_length);
int find_next_token_from(token_kind type, int index, list_t *tok_list);

int get_next_after_whitespace(int start_index, list_t *tok_list);

bool check_if_whitespace_tok(token_t t);
bool check_if_whitespace_char(char c);

// Remove all tokens with the specified type.
void remove_all_tokens_by_type(token_list_t *list, token_kind type);

#ifdef __cplusplus
}
#endif

#endif // _LEXER_UTIL_H