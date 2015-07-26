
#include "includes.h"


/* 
 * This whole piece of crap file and its functions are all really messy (and probably poorly coded).
 * One day, convert this and lang.txt to xml (libxml2)...
 *
 *
 *
 *
 * One day...
 */



// A structure holding two malloc'd string buffers.
// It defines a "replace" macro
static struct rmacro {
	char *name;
	list_t *token_str;
};



// Only to be used if the values in the macro structure are malloc'd. Otherwise, there is undefined behavior from the free() function.
static void _free_macro_struct(struct rmacro *macro) {
	free(macro->name);
	list_free(macro->token_str, NULL);
}



/*
 * Returns a copy of the original string, and changes buf and length to the new buffer and the length of the new buffer, respectively.
 *
 * TODO: Make this function take a token list as an input, and return a token_list
 */
static list_t *evaluate_macros(list_t *tok_list) {

	int i;

	list_t *macro_list = list_alloc(5, sizeof(struct rmacro));

	for (i = 0; i < tok_list->length; i++) {
		token_t cur_tok = GET_TOKEN_AT(tok_list, i);
		char *cur_tok_str = tok_to_str(cur_tok);


		if (cur_tok.type == token_id && STREQ(MACRO_REPLACE, cur_tok_str)) {

			// Obtain the token representing the name of the macro
			// We increment by one

			i = get_next_after_whitespace(++i, tok_list);
			token_t name_tok = GET_TOKEN_AT(tok_list, i);

			// Advance past the name token
			i = get_next_after_whitespace(++i, tok_list);

			// Update the cur_tok again
			free(cur_tok_str);
			cur_tok = GET_TOKEN_AT(tok_list, i);
			cur_tok_str = tok_to_str(cur_tok);

			int macro_end = find_next_token_from(token_newline, i, tok_list);

			// Create macro structure
			struct rmacro *macro = MALLOC_ONE(struct rmacro);
			macro->name = tok_to_str(name_tok);

			// i is the beginning, macro_end is the end
			list_t *macro_tok_str = list_alloc(macro_end - i, sizeof(token_t));
			for (; i < macro_end; i++) {
				list_add_item(&(GET_TOKEN_AT(tok_list, i)), macro_tok_str);
			}

			// i is now equal to macro_end

			// Now that all the items are added:
			macro->token_str = macro_tok_str;

			list_add_item(macro, macro_list);

			free(macro);

			goto loop_end;


		}

		
		int j;
		struct rmacro *cur_macro;
		for (j = 0; j < macro_list->length; j++) {
			cur_macro = (struct rmacro *)GET_ITEM_AT(macro_list, j);
			if (STREQ(cur_macro->name, cur_tok_str)) {

				int k = 0;
				int tmp_index = i;
				
				list_remove_item(tmp_index, tok_list);

				
				// Now insert the rest of the tokens
				for (; k < cur_macro->token_str->length; k++) {
					list_insert_item(GET_ITEM_AT(cur_macro->token_str, k), tmp_index, tok_list);
					tmp_index++;
				}
				
				i--;
				goto loop_end;
			}
		}
		

	loop_end:
		free(cur_tok_str);
		;
	}

	// Clean up

	list_free(macro_list, &_free_macro_struct);

	list_t *ret = tok_list;
	return ret;
}



/* 
 * Removes comments and replaces macros. We assume comments are on their own lines for now.
 * Returns a pointer to the new token list that was generated, and buf remains intact
 * 
 * TODO: Change char *buf in the parameter list to a token list, as this function now returns a token list
 */
/* static */ list_t *lang_txt_preprocess(const char *buf, int length) {
	scanner_t *scan = scanner_alloc();
	scan_buffer(buf, length, scan); 
	list_t *tok_list = scan->token_list;

	int i;

	// Now that we have a token list, we remove all \r characters on windows, 
	// effectively converting \r\n to \n (Unix only has \n for newline).
#ifdef _WINDOWS

	for (i = 0; i < tok_list->length; i++) {
		if (GET_TOKEN_AT(tok_list, i).type == token_cr) {
			list_remove_item(i--, tok_list);
		}
	}

#endif

	for (i = 0; i < tok_list->length; i++) {
		if (GET_TOKEN_AT(tok_list, i).type == token_newline) {
			i = get_next_after_whitespace(++i, tok_list);
			if (i == -1) {
				// EOF is reached, break.
				break;
			}
			if (GET_TOKEN_AT(tok_list, i).type == COMMENT_CHAR) {	// token_hash
				int token_end = find_next_token_from(token_newline, i, tok_list);
				int tmp = i;
				for (; tmp < token_end; tmp++) {
					list_remove_item(i, tok_list);
				}
				// Just in case two comments are back to back.
				i--;
			}
		}
	}

	// Now evaluate macros
	evaluate_macros(tok_list);

	// puts(new_buf);
	//scanner_free(scan, false);

	

	return tok_list;
}

lang_state_t *lang_txt_parse_buffer(const char *buf, int length) {
	
	// lang_state_t *ret = (lang_state_t *)malloc(sizeof(lang_state_t));
	// Gradually start changing the malloc statements:
	lang_state_t *ret = MALLOC_ONE(lang_state_t);
	
	list_t *tok_list = lang_txt_preprocess(buf, length);

	ret->tok_list = tok_list;

	int i;

	for (i = 0; i < tok_list->length; i++) {
		if (GET_TOKEN_AT(tok_list, i).type == '[') {
			if (STREQ(tok_to_str(GET_TOKEN_AT(tok_list, i + 1)), SCANNER_TAG) 
				&& GET_TOKEN_AT(tok_list, i + 2).type == ']') {	
				i += 2;
				ret->scanner_index = i;
			}

			else if (STREQ(tok_to_str(GET_TOKEN_AT(tok_list, i + 1)), PARSER_TAG)
				&& GET_TOKEN_AT(tok_list, i + 2).type == ']') {
				i += 2;
				ret->parser_index = i;
			}
			else if (STREQ(tok_to_str(GET_TOKEN_AT(tok_list, i + 1)), CFG_TAG)
				&& GET_TOKEN_AT(tok_list, i + 2).type == ']') {
				i += 2;
				ret->cfg_index = i;
			}
			else if (STREQ(tok_to_str(GET_TOKEN_AT(tok_list, i + 1)), ILCGEN_TAG)
				&& GET_TOKEN_AT(tok_list, i + 2).type == ']') {
				i += 2;
				ret->ilcgen_index = i;
			}
			else if (STREQ(tok_to_str(GET_TOKEN_AT(tok_list, i + 1)), CGEN_TAG)
				&& GET_TOKEN_AT(tok_list, i + 2).type == ']') {
				i += 2;
				ret->cgen_index = i;
			}
			else {
				// Report unknown tag
			}
		}

		//from here on we don't need anything.
	}

	return ret;
}



cfg_t *lang_get_cfg(lang_state_t *state) {

	int i;

	cfg_t *ret = cfg_alloc();

	list_t *tok_list = state->tok_list;

	int start_index = state->cfg_index;
	
	/* We added 2 to the the index when we initialized it, and i was incremented again, so we subtract 3
	 * to rewind to before the opening [ which would cause errors in cfg_parse.
	 */
	int end_index = state->ilcgen_index - 3;

	for (i = start_index; i < tok_list->length; i++) {
		if (i >= end_index) break;
		if (GET_TOKEN_AT(tok_list, i).type == token_id
			&& GET_TOKEN_AT(tok_list, get_next_after_whitespace(i + 1, tok_list)).type == ':') {

			// Sets i to end of cfg index if success, returns NULL if fail.
			i = RETURN_VAL_IF_ERROR(cfg_parse_one(tok_list, i, ret), NULL);
		}
	}

	return ret;

}

void lang_free(lang_state_t *state) {
	list_free(state->tok_list, NULL);
	free(state);
}