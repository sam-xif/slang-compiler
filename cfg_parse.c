
#include "includes.h"

/*
 * Parse a cfg object defined in the language file.
 * This only returns a cfg with the match info set. The caller has to set the start_symbol.
 * It only fills the structure with relevant data.
 *
 * Parses one declaration up to its terminating semicolon, and returns.
 *
 * Returns the index of the token after the semicolon, or -1 if error.
 */
int cfg_parse_one(list_t *tok_list, int start_index, cfg_t *cfg) {
	
	// cfg is assumed to be a valid pointer to an initialized cfg structure.
	// Parsing starts at the start index and stops at next semicolon found.

	// Loop counter
	int i = start_index;

	// Allocate rule
	cfg_rule_t *rule = cfg_rule_alloc();

	// Copy the start symbol so that it is independent from the list.
	token_t *start_symbol = MALLOC_ONE(token_t);
	*start_symbol = *(GET_ITEM_AS(tok_list, i, token_t));

	*(rule->start_symbol) = *start_symbol;

	// The list should never contain the start symbol, so we use assert instead of if.
	assert(!list_contains_item(start_symbol, cfg->terminals, &cfg_symbol_compare_strict));

	list_add_item(start_symbol, cfg->start_symbols);
	
	free(start_symbol);

	// Call twice to get to the start of the cfg rule list.
	i = get_next_after_whitespace(++i, tok_list);
	i = get_next_after_whitespace(++i, tok_list);

	// i now points to the beginning of the cfg content.

	// The list that will be placed into the return structure.
	list_t *ret_list = LIST_CREATE(match_list_t);

	// Temporary list that stores match values until the next separator (vertical pipe '|') is encountered.
	match_list_t *tmp_list = MALLOC_ONE(match_list_t);
	tmp_list->matches = LIST_CREATE(match_t);

	// This is set to true by default.
	tmp_list->is_terminal = true;

	// true = terminal, false = non-terminal
	bool terminal = false;

	// A valid token could be: $(+)(1-*). This means "match the + sign at least 1 time."
	for (; i < tok_list->length; i++) {

		if (GET_TOKEN_AT(tok_list, i).type == CFG_RULE_SEPARATOR) {
			// Cut off the list and add to return structure.
			list_add_item(tmp_list, ret_list);

			// Free the old list and create a new one.
			// Cannot free like this because it destroys the array
			//list_free(tmp_list, NULL);
			// Free like this:
			free(tmp_list);
			tmp_list = MALLOC_ONE(match_list_t);
			tmp_list->matches = LIST_CREATE(match_t);
			tmp_list->is_terminal = true;
			continue;
		}
		else if (GET_TOKEN_AT(tok_list, i).type == CFG_RULE_TERMINATOR) {
			list_add_item(tmp_list, ret_list);

			// Free the old list and create a new one.
			free(tmp_list);

			break;
		}

		token_t *match_token = MALLOC_ONE(token_t);
		if (GET_TOKEN_AT(tok_list, i).type == '$') {
			// Pre-Increment i to skip to next token
			if (GET_TOKEN_AT(tok_list, ++i).type != '(') {
				puts("Syntax error in language file.");
				errno = SLANG_SYNTAX_ERROR;
				// lang.txt for now. Eventually pass filename as parameter
				raise_error("lang.txt", GET_TOKEN_AT(tok_list, i), NULL);
				return -1;
			}
			

			// Increment i again to get to next token.
			if (GET_TOKEN_AT(tok_list, ++i).type == token_integer) {
				// Create the token with only the type specified
				char *tmp = tok_to_str(GET_TOKEN_AT(tok_list, i));
				char **endptr = NULL;
				match_token->type = (token_kind)strtol(tmp, endptr, 10);
				match_token->data_ptr = NULL;
				match_token->index_in_buf = 0;
				match_token->data_len = 0;
				free(tmp);
			}
			else {
				*match_token = GET_TOKEN_AT(tok_list, i);
			}

			if (GET_TOKEN_AT(tok_list, ++i).type != ')') {
				puts("Syntax error in language file.");
				errno = SLANG_SYNTAX_ERROR;
				// lang.txt for now. Eventually pass filename as parameter
				raise_error("lang.txt", GET_TOKEN_AT(tok_list, i), NULL);
				return -1;
			}

			terminal = true;
		}
		else if (GET_TOKEN_AT(tok_list, i).type == token_id) {			
			*match_token = GET_TOKEN_AT(tok_list, i);
			terminal = false;
		}
		else if (check_if_whitespace_tok(GET_TOKEN_AT(tok_list, i))) {
			continue;
		}
		else {
			// We assume its a character of some sort
			match_token->type = GET_TOKEN_AT(tok_list, i).type;
			match_token->data_ptr = NULL;
			match_token->data_len = 0;
			terminal = true;
		}

		// Maybe don't malloc here..
		match_t *match = MALLOC_ONE(match_t);
		match->error_code = SLANG_NO_ERROR;
		match->error_msg = NULL;

		if (GET_TOKEN_AT(tok_list, ++i).type == '[') {
			if (GET_TOKEN_AT(tok_list, ++i).type == token_id) {
				// We have a tag
				// Advance index
				const char *tag_name = tok_to_str(GET_TOKEN_AT(tok_list, i));
				if (tag_name == NULL) return -1;
				if (STREQ(tag_name, CFG_ERROR_TAG)) {
					// Error tag
					// Advance index again
					i = get_next_after_whitespace(++i, tok_list);
					
					if (GET_TOKEN_AT(tok_list, i).type != token_integer) {
						errno = SLANG_SYNTAX_ERROR;
						raise_error("lang.txt", GET_TOKEN_AT(tok_list, i), NULL);
						return -1;
					}

					const char *error_code_str = tok_to_str(GET_TOKEN_AT(tok_list, i));
					char **endptr = NULL;
					int error_code = (int)strtol(error_code_str, endptr, 10);

					match->error_code = error_code;

					i = get_next_after_whitespace(++i, tok_list);

					if (GET_TOKEN_AT(tok_list, i).type != token_string) {
						errno = SLANG_SYNTAX_ERROR;
						raise_error("lang.txt", GET_TOKEN_AT(tok_list, i), NULL);
						return -1;
					}

					const char *error_msg = tok_to_str(GET_TOKEN_AT(tok_list, i));
					match->error_msg = error_msg;
				}
				else {
					errno = SLANG_SYNTAX_ERROR;
					raise_error("lang.txt", slang_create_error_token(0, 0), NULL);
					return -1;
				}

				// Advance one more time
				i = get_next_after_whitespace(++i, tok_list);
				if (GET_TOKEN_AT(tok_list, i).type != ']') {
					errno = SLANG_SYNTAX_ERROR;
					raise_error("lang.txt", slang_create_error_token(0, 0), NULL);
					return -1;
				}
				
				free(tag_name);
			}
		}
		else {
			i--;
		}

		if (terminal == true) {
			if (!list_contains_item(match_token, cfg->terminals, &cfg_symbol_compare_strict)) {
				list_add_item(match_token, cfg->terminals);
			}
		}
		else {
			if (!list_contains_item(match_token, cfg->non_terminals, &cfg_symbol_compare_strict)) {
				list_add_item(match_token, cfg->non_terminals);
			}
		}

		match->is_terminal = terminal;
		
		if (terminal == false) tmp_list->is_terminal = false;

		// To prevent uninitialized variable use error.

		// The above code could be converted to:
		//RETURN_VAL_IF_ERROR( MEMCPY(&(match->match_token), sizeof(token_t), match_token, sizeof(token_t)) , -1);
		match->match_token = *match_token;
		// list_add_item memcpy's each element, so no worry about corruption (for now, at least).
		list_add_item(match, tmp_list->matches);

		free(match);
		free(match_token);
	}

	// Epilogue

	rule->match_data = ret_list;
	list_add_item(rule, cfg->rule_list);

	// Clean up a bit
	free(rule);

	// Return index to the next token after the semicolon.
	return (++i);
}


match_list_t *match_list_alloc() {
	match_list_t *ret = MALLOC_ONE(match_list_t);
	ret->matches = list_alloc(LIST_DEFAULT_CAPACITY, sizeof(match_t));
	return ret;
}


void match_list_free(match_list_t *list) {
	if (list == NULL) return;

	list_free(list->matches, NULL);
	free(list);
}