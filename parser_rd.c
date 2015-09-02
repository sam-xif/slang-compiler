
#include "includes.h"


/*
* PARSER TODO:
* Make it smart so that infinite loops do not occur (i.e. the same two elements calling each other. Example:
*
*		expr : add;
*		add : expr + expr | (add);
*
* The above will cause an infinite loop. To avoid this, create a way to skip to the next part of the rule
* (which would be (add) in this case) if there is one.
* Or simply return an error instead of allowing stack overflow.
*
* Also add the same while-loops to non-terminals so they can be matched multiple times.
*
* Other than that is seems to work decently well
*/


/* ====== PASTED FROM PARSER BACKUP ====== */


// Prototype needed to alleviate errors.
static bool _process_rule(cfg_rule_t *cur_rule, parser_t *parse, st_node_t *parent);


// If output is true, the parse->index is set to the next token to be matched. 
// If false, parse->index is the same as input
// This function is guaranteed not to modify parse->save_index.
static bool _check_match(match_t *match, match_t *next_match, parser_t *parse,
	st_node_t *parent, int *out_times_matched, bool *out_is_terminal) {

	int save_index = parse->index;

	// TODO: replace all token lists with token_list_t
	token_list_t *tok_list = parse->token_list;

	int i = parse->index;
	int num_times_matched = 0;
	if (i == tok_list->length) goto return_true;


	if (match->match_token.type == token_epsilon) {
		// Immediately return true if epsilon is being matched, since it always matches with everything.
		// We don't have to advance index either, since epsilon represents the empty string.
		st_node_t *tmp_child = MALLOC_ONE(st_node_t);
		tmp_child->type = NODE_LEAF;
		tmp_child->parent = parent;
		tmp_child->data = match->match_token;
		tmp_child->children = list_alloc(NODE_LIST_DEFAULT_CAPACITY, sizeof(st_node_t *));
		tmp_child->child_index = parent->children->length;

		list_add_item(&tmp_child, parent->children);

		num_times_matched++;

		goto return_true;
	}

	if (cfg_is_start_symbol(&(match->match_token), parse->grammar)) {
		const char *name = tok_to_str(match->match_token);
		st_node_t *tmp_child = MALLOC_ONE(st_node_t);

		tmp_child->type = NODE_NT;
		tmp_child->parent = parent;
		tmp_child->data = match->match_token;
		tmp_child->children = list_alloc(NODE_LIST_DEFAULT_CAPACITY, sizeof(st_node_t *));
		tmp_child->child_index = parent->children->length;
		// Assumes parent has a list allocated
		list_add_item(&tmp_child, parent->children);


		// TODO: Add code to process this more than one time based on the bound indicators
		bool result = _process_rule(cfg_get_rule_by_name(name, parse->grammar), parse, tmp_child);
		free(name);

		*out_is_terminal = false;
		if (result == true) {
			(*out_times_matched)++;
			*out_is_terminal = false;
			return true;
		}
		else {
			*out_times_matched = 0;
			return false;
		}
	}

	// Set default lower and upper bounds.
	int upper_bound = 1;
	int lower_bound = 1;

	while (cfg_symbol_compare(GET_ITEM_AS(tok_list, i, token_t), &(match->match_token))
		&& i < tok_list->length
		&& num_times_matched < upper_bound) {

		// Insert code to add to list here
		st_node_t *tmp_child = MALLOC_ONE(st_node_t);
		tmp_child->type = NODE_LEAF;
		tmp_child->data = GET_TOKEN_AT(tok_list, i);
		tmp_child->parent = parent;
		tmp_child->children = NULL;
		tmp_child->child_index = parent->children->length;

		list_add_item(&tmp_child, parent->children);

		i++;
		num_times_matched++;
	}

	if (num_times_matched < lower_bound) {
		goto return_false;
	}

	goto return_true;

return_true:

	parse->index = i;
	*out_times_matched = num_times_matched;
	*out_is_terminal = true;
	return true;


return_false:

	parse->index = save_index;
	*out_is_terminal = true;
	return false;
}

bool _process_rule(cfg_rule_t *cur_rule, parser_t *parse, st_node_t *parent) {
	st_node_t *save_node = MALLOC_ONE(st_node_t);
	*save_node = *parent;
	save_node->children = NULL;

	bool ret = false;

	bool has_backup = false;

	// Save the original starting point
	int origin_index = parse->index;
	int tmp_index = origin_index;

	int i;
	for (i = 0; i < cur_rule->match_data->length; i++) {
		int j;
		match_list_t *cur_match_list = (GET_ITEM_AS(cur_rule->match_data, i, match_list_t));
		int save_index = parse->index;
		for (j = 0; j < cur_match_list->matches->length; j++) {
			int k;
			int times_matched;
			bool is_terminal;

			bool result = _check_match(GET_ITEM_AS(cur_match_list->matches, j, match_t),
				j + 1 < cur_match_list->matches->length
				? GET_ITEM_AS(cur_match_list->matches, j + 1, match_t)
				: NULL, parse, parent, &times_matched, &is_terminal);

			parse->save_result = result;

			if (result == true) {
				// Set status to true and continue matching
				ret = true;
			}
			else {
				// Check if should throw error:
				if (GET_ITEM_AS(cur_match_list->matches, j, match_t)->error_code != 0
					/* Suppress errors at the end of the stream. */
					&& GET_TOKEN_AT(parse->token_list, parse->index).type != token_end_of_stream) {
					char *error_token = tok_to_str(GET_TOKEN_AT(parse->token_list, parse->index));

					// Add 5 for " at \"", add 2 for other double quote and null terminator
					int dest_size = 0;
					if (GET_ITEM_AS(cur_match_list->matches, j, match_t)->error_msg != NULL)
						dest_size = 5 + strlen(error_token) + 3 + strlen(GET_ITEM_AS(cur_match_list->matches, j, match_t)->error_msg) + 1;
					else
						dest_size = 5 + strlen(error_token) + 2;
					char *dest = MALLOC(dest_size, char);

					STRCPY(dest, dest_size, " at \"");
					STRCAT(dest, dest_size, error_token);
					if (GET_ITEM_AS(cur_match_list->matches, j, match_t)->error_msg != NULL) {
						STRCAT(dest, dest_size, "\":");
						STRCAT(dest, dest_size, GET_ITEM_AS(cur_match_list->matches, j, match_t)->error_msg);
					}
					else
						STRCAT(dest, dest_size, "\"");

					// Save the current errno value.

					// No need to save errno because the error flag has_error is set when an error is raised. That should be the error indicator
					//errno_t save_errno = errno;

					errno = GET_ITEM_AS(cur_match_list->matches, j, match_t)->error_code;
					raise_error("unknown", GET_TOKEN_AT(parse->token_list, parse->index), dest);

					// Restore errno
					//errno = save_errno;

					// Clean up
					free(dest);
					free(error_token);
				}

				// Restore original state
				parse->index = save_index;

				parser_free_st(parent, false);
				*parent = *save_node;
				parent->children = list_alloc(NODE_LIST_DEFAULT_CAPACITY, sizeof(st_node_t *));

				// Set status to false and break out of loop
				ret = false;
				break;
			}
		}
		if (ret == true || parse->index == parse->token_list->length) {
			return ret;
		}
	}

	return ret;
}





// Sets the parser token list to the tok_list parameter by default
// Returns NULL on error or if grammar doesn't match input
st_node_t *parse_recursive_descent(token_list_t *tok_list, parser_t *parse) {
	parse->token_list = tok_list;
	parse->state = PARSE_WORKING;

	// Creates the root node of the parse tree.
	st_node_t *root = MALLOC_ONE(st_node_t);
	root->parent = NULL; 
	root->child_index = 0;
	root->children = list_alloc(NODE_LIST_DEFAULT_CAPACITY, sizeof(st_node_t *));
	root->type = NODE_ROOT;

	list_t *rule_list = parse->grammar->rule_list;

	/*
	 * TODO: Add error printing so that the user can have a more interactive experience.
	 * Add facilities into the _process_rule and _check_match functions to print errors
	 * when a false is returned.
	 */


	root->data = *(GET_ITEM_AS(rule_list, 0, cfg_rule_t)->start_symbol);
	if (_process_rule(GET_ITEM_AS(rule_list, 0, cfg_rule_t), parse, root) == true) {
		if (GET_ITEM_AS(tok_list, parse->index, token_t)->type == token_end_of_stream) {
			parse->state = PARSE_FINISH;
			parse->error = SLANG_NO_ERROR;
			parse->result = root;
			return root;
		}
	}
		
	

	parse->state = PARSE_ERR;
	parse->error = SLANG_PARSE_REJECTED;
	errno = parse->error;
	raise_error("unknown", slang_create_error_token(0, 0), NULL);
	parse->result = NULL;
	return NULL;
}


bool _rd_rewriter_selector(st_node_t *node, bool *usable) {
	if (node->type == NODE_NT) {
		const char *node_name = tok_to_str(node->data);
		if (node_name == NULL) {
			// Error.
			*usable = false;
			return false;
		}

		int name_len = strlen(node_name);

		if (node_name[name_len - 1] == '_') {
			// This means it is a left factored node. Join its children with the parent's.

			// Removes this node from the parent's children.
			list_remove_item(node->child_index, node->parent->children);

			// Loop through the children, and add them to the parent's children in the missing spot
			int i;

			for (i = node->children->length - 1; i >= 0; i--) {
				list_insert_item(GET_ITEM_AT(node->children, i), node->child_index, node->parent->children);
				(*GET_ITEM_AS(node->parent->children, node->child_index, st_node_t *))->parent = node->parent;
			}

			for (i = 0; i < node->parent->children->length; i++) {
				(*GET_ITEM_AS(node->parent->children, i, st_node_t *))->child_index = i;
			}

			// Release the node from memory, since it is now completely removed from the tree.
			node->children = NULL;
			node->child_index = 0;
			node->parent = NULL;
			free(node);
			*usable = false;
		}
		else {
			// TODO: Put all these messages into a slang errno function.
			//errno_t save_errno = errno;
			errno = SLANG_BAD_PARSE_TREE;
			raise_error("parser_rd.c", slang_create_error_token(305, 0), NULL);
			//errno = save_errno;
		}

		free(node_name);
	}
	return false;
}


st_node_t *parse_rd_rewrite_tree(st_node_t *root) {
	/*
	 * In this function, move the left factor children onto the parent nodes.
	 * Just create a static selector function and call pt_select_nodes.
	 */
	list_free(st_select_nodes(root, &_rd_rewriter_selector), NULL);
	return root;
}
