
#include "includes.h"


/*
* Exactly like the function above, but without the need to pass a scanner object.
* buf cannot be NULL, and buf_length cannot be 0.
*
* Maybe implement this such that it will skip over c if it is escaped by ESCAPE_CHAR?
*/
int find_next_char_from(char c, int index, char *buf, int buf_length) {
	int _index;

	if (buf == NULL) { errno = EINVAL; return -1; }
	if (buf_length <= 0) { errno = EINVAL; return -1; }

	for (_index = index; _index < buf_length; _index++) {
		if (buf[_index] == c) break;
	}

	return _index;
}


int find_next_token_from(token_kind type, int index, list_t *tok_list) {
	int _index;

	if (tok_list == NULL) { errno = EINVAL; return -1; }
	if (type == token_undefined) { errno = EINVAL; return -1; }

	for (_index = index; _index < tok_list->length; _index++) {
		if (GET_TOKEN_AT(tok_list, _index).type == type) break;
	}

	return _index;
}


// returns true or false
bool check_if_whitespace_tok(token_t t) {

#ifdef WHITESPACE
	char *whitespace = WHITESPACE;
	int ws_len = strlen(whitespace);
	int i;

	// Here, we rely on the fact that get_token() doesn't lie...
	if (t.type == token_whitespace) return true;

	if (t.type >= 0 && t.type <= 128) {
		for (i = 0; i < ws_len; i++) {
			if (whitespace[i] == t.type) {
				return true;
			}
		}
	}
	else {
		// This isn't exactly an error, and its causing problems, so comment it out
		//puts("Error: The token passed is not a valid ASCII character");
		//errno = ENOENT;
		return false;
	}
	return false;

#else
	puts("Error: WHITESPACE is not defined");
	exit(1);
#endif
}

bool check_if_whitespace_char(char c) {

#ifdef WHITESPACE
	char *whitespace = WHITESPACE;
	int ws_len = strlen(whitespace);
	int i;


	// We assume ASCII in these until Unicode versions of these functions are written
	if (c >= 0 && c <= 128) {
		for (i = 0; i < ws_len; i++) {
			if (whitespace[i] == c) {
				return true;
			}
		}
	}
	else {

		// This isn't exactly an error
		//puts("Error: The character passed is not a valid ASCII character");
		//errno = ENOENT;
		return false;
	}
	return false;

#else
	puts("Error: WHITESPACE is not defined");
	exit(1);
#endif
}

/*
* start_index points to the first token you want to check. If start_index points to a non_whitespace character,
* this function will return start_index.
* Returns -1 if it reached EOF before finding a non-whitespace token, and returns -2 if some other error occurred.
*/
int get_next_after_whitespace(int start_index, list_t *tok_list) {
	int i;



	for (i = start_index; i < tok_list->length; i++) {
		bool b = check_if_whitespace_tok(GET_TOKEN_AT(tok_list, i));
		if (errno != NOERROR) {
			return -2;
		}
		else {
			if (b == false) {
				return i;
			}
		}
	}

	return -1;
}


void remove_all_tokens_by_type(token_list_t *list, token_kind type) {
	assert(list != NULL);

	int i;
	for (i = 0; i < list->length; i++) {
		if (GET_ITEM_AS(list, i, token_t)->type == type) {
			list_remove_item(i, list);
			i--;
		}
	}
}
