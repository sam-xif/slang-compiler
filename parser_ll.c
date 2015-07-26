
#include "includes.h"

pt_node_t *parse_stack(token_list_t *tok_list, parser_t *parse) {
	parse->token_list = tok_list;
	parse->state = PARSE_WORKING;
	parse->symbol_stack = stack_alloc(LIST_DEFAULT_CAPACITY, sizeof(match_t));

	bool stack_has_terminals = false;

	/* Prepare the stack for parsing */

	match_t eos_match;
	eos_match.is_terminal = true;

	// Create an end of stream token.
	token_t eos;
	eos.type = token_end_of_stream;
	eos.data_ptr = NULL;
	eos.data_len = 0;
	eos.index_in_buf = 0;

	eos_match.match_token = eos;

	// Pushes the end of stream symbol onto the stack.
	stack_push(&eos_match, parse->symbol_stack);
	
	list_t *rule_list = parse->grammar->rule_list;

	match_t start_symbol_match;
	start_symbol_match.is_terminal = false;
	start_symbol_match.match_token = *(GET_ITEM_AS(rule_list, 0, cfg_rule_t)->start_symbol);

	// Push start symbol onto stack:
	stack_push(&start_symbol_match, parse->symbol_stack);

	while (((match_t *)stack_peek(parse->symbol_stack))->match_token.type != token_end_of_stream) {
		cfg_rule_t *cur_rule;

		// Pop the top symbol
		match_t *top_symbol = stack_pop(parse->symbol_stack);

		// Replace this call to cfg_is_non_terminal when we use a data structure that contains an is_terminal boolean.
		if (cfg_is_terminal(&(top_symbol->match_token), parse->grammar)) {
			// Match with input
		}
		else if (cfg_is_non_terminal(&(top_symbol->match_token), parse->grammar)) {

			const char *name = tok_to_str(top_symbol->match_token);
			cur_rule = cfg_get_rule_by_name(name, parse->grammar);

			int i;
			for (i = 0; i < cur_rule->match_data->length; i++) {
				int j;
				match_list_t *cur_match_list = (GET_ITEM_AS(cur_rule->match_data, i, match_list_t));
				parse->save_index = parse->index;

				// Push items onto the stack in reverse order so that the first match is first to be popped.
				for (j = cur_match_list->matches->length - 1; j >= 0; j--) {
					match_t *next_push = GET_ITEM_AS(cur_match_list->matches, j, match_t);
					stack_has_terminals |= next_push->is_terminal;
					stack_push(next_push, parse->symbol_stack);
				}



				if (GET_ITEM_AS(cur_match_list->matches, j, match_t)->is_terminal == true) {

				}
			}

			free(name);

		}
		else {
			// Error?
		}

	}
	
	return NULL;

}