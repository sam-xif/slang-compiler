
#include "includes.h"

static void _free_st_func(st_node_t *node) {
	if (node == NULL) return;

	// Possibly replace with checking the node_type instead, as its more reliable
	if (node->type != NODE_LEAF) {
		int i;
		for (i = 0; i < node->children->length; i++) {
			// Recurse

			// A pointer to a pointer to a node
			st_node_t **child = GET_ITEM_AS(node->children, i, st_node_t *);

			if ((*child)->type != NODE_LEAF) {
				_free_st_func(*child);
			}

			// Dereference only once to obtain the pointer to the node and free it
			free(*child);
		}
	}

	// list_free(node->children, &parser_free_pt);
	/*
	* We don't need to free the node pointer because the data it represents is encapsulated
	* in the parent list array, which will be freed eventually.
	*/
}

void parser_free_st(st_node_t *root, bool free_root) {
	if (root == NULL) return;

	if (root->type != NODE_LEAF) {
		_free_st_func(root);
	}

	list_free(root->children, NULL);

	if (free_root)
		free(root);
}


parser_t *parser_alloc(cfg_t *grammar) {
	parser_t *ret = MALLOC_ONE(parser_t);
	ret->state = PARSE_START;
	ret->grammar = grammar;
	ret->index = 0;
	ret->result = NULL;
	ret->state = 0;
	ret->token_list = NULL;
	ret->symbol_stack = NULL;
	return ret;
}


void parser_free(parser_t *p, bool free_cfg) {
	if (p == NULL) return;

	if (free_cfg)
		cfg_free(p->grammar);

	parser_free_st(p->result, true);
	list_free(p->token_list, NULL);
	free(p);
}

static bool _get_leaves_selector(st_node_t *node, bool *usable) {
	if (node->type == NODE_LEAF) return true;
	else return false;
}

node_list_t *st_get_leaves(st_node_t *node) {
	if (node->type == NODE_LEAF) {
		puts("Unable to get the leaves of a node with no children.");
		errno = EINVAL;
		return NULL;
	}

	return st_select_nodes(node, &_get_leaves_selector);
}



// Implemented mainly for debugging purposes now, but will be useful later.
// The selector function pointer can be NULL if one wants every single node of the tree to be returned.
node_list_t *st_select_nodes(st_node_t *root, bool(*selector_func)(st_node_t *, bool *)) {
	assert(root != NULL);

	node_list_t *ret = (node_list_t *)LIST_CREATE(st_node_t);
	bool usable = true;

	// First check the root node:
	if (selector_func == NULL) list_add_item(root, ret);
	else if (selector_func(root, &usable) == true) list_add_item(root, ret);

	if (usable == true) {
		if (root->type != NODE_LEAF && root->children != NULL) {
			int i;
			for (i = 0; i < root->children->length; i++) {
				// We cast to st_node_t* because pointers to pointers to nodes are stored in the children arrays.
				// This is also why we dereference once, so that we can obtain the pointer to the data, not the pointer to the pointer.
				node_list_t *result = st_select_nodes(*(GET_ITEM_AS(root->children, i, st_node_t *)), selector_func);
				list_concatenate(ret, result);
				list_free(result, NULL);
			}
		}
	}

	return ret;
}


node_list_t *st_parameterized_select_nodes(st_node_t *root, void *data, bool(*selector_func)(st_node_t *, bool *, void *)) {
	assert(root != NULL);

	node_list_t *ret = (node_list_t *)LIST_CREATE(st_node_t);
	bool usable = true;

	// First check the root node:
	if (selector_func == NULL) list_add_item(root, ret);
	else if (selector_func(root, &usable, data) == true) list_add_item(root, ret);

	if (usable == true) {
		if (root->type != NODE_LEAF && root->children != NULL) {
			int i;
			for (i = 0; i < root->children->length; i++) {
				// We cast to st_node_t* because pointers to pointers to nodes are stored in the children arrays.
				// This is also why we dereference once, so that we can obtain the pointer to the data, not the pointer to the pointer.
				node_list_t *result = st_parameterized_select_nodes(*(GET_ITEM_AS(root->children, i, st_node_t *)), 
					data, selector_func);
				list_concatenate(ret, result);
				list_free(result, NULL);
			}
		}
	}

	return ret;
}


static bool _get_nodes_by_name_selector(st_node_t *node, bool *usable, void *data) {
	const char *name = (const char *)data;
	const char *node_name = tok_to_str(node->data);
	*usable = true;
	if (node_name != NULL) {
		if (STREQ(name, node_name)) {
			free(node_name);
			return true;
		}
		else {
			free(node_name);
			return false;
		}
	}
	else {
		free(node_name);
		return false;
	}
}


node_list_t *st_get_nodes_by_name(st_node_t *root, const char *name) {
	return st_parameterized_select_nodes(root, (void *)name, &_get_nodes_by_name_selector);
}


static struct _get_first_data_struct {
	const char *name;
	unsigned int ret_length;
};


static bool _get_first_by_name_selector(st_node_t *node, bool *usable, void *data) {
	struct _get_first_data_struct *data_struct = (struct _get_first_data_struct *)data;
	if (data_struct->ret_length == 1) return false;

	const char *name = data_struct->name;
	const char *node_name = tok_to_str(node->data);
	if (node_name != NULL) {
		if (STREQ(name, node_name)) {
			free(node_name);
			data_struct->ret_length++;
			return true;
		}
		else {
			free(node_name);
			return false;
		}
	}
	else return false;
}


st_node_t *st_get_first_by_name(st_node_t *root, const char *name) {
	struct _get_first_data_struct data;
	data.name = name;
	data.ret_length = 0;
	node_list_t *result = st_parameterized_select_nodes(root, (void *)(&data), &_get_first_by_name_selector);
	if (result->length > 1) {
		errno = SLANG_GENERIC_ERROR;
		raise_error("parser_defs.c", slang_create_error_token(204, 0), NULL);
	}
	return GET_ITEM_AS(result, 0, st_node_t);
}


st_node_t *parse_cst_to_ast(st_node_t *root) {

	// Create the root node of the tree
	st_node_t *ret = MALLOC_ONE(st_node_t);

	// Initialize some values
	ret->type = NODE_ROOT;
	ret->parent = NULL;
	ret->children = LIST_CREATE(st_node_t);
	ret->child_index = 0;

	// Zero out token structure
	memset(&ret->data, 0, sizeof(token_t));

	// Obtain all the aliases that are defined in the parse tree
	node_list_t *aliases = st_get_nodes_by_name(root, "ALIAS_DIRECTIVE");

	// Obtain the name of the namespace that the names should be placed in.
	node_list_t *file_ns = st_get_nodes_by_name(root, "NS_DIRECTIVE");

	// Make sure only one namespace is declared
	assert(file_ns->length <= 1);

	// Get all the functions defined in the parse tree.
	node_list_t *functions = st_get_nodes_by_name(root, "FUNCTION");

	// Get all the attribute tags in the file
	node_list_t *tags = st_get_nodes_by_name(root, "TAG");

	// Get all the linker references made
	node_list_t *references = st_get_nodes_by_name(root, "REFERENCE_DIRECTIVE");

	// Get all the use directives in the tree
	node_list_t *use_directives = st_get_nodes_by_name(root, "USE_DIRECTIVE");

	// Get all the function definitions in the tree.
	node_list_t *func_defs = st_get_nodes_by_name(root, "FUNCDEF");




	return ret;
}


st_node_t *_create_ast_node(st_node_t *root, cst_node_kind type) {

}


cst_node_kind parse_get_cst_node_kind(st_node_t *node) {
	assert(node->type != NODE_LEAF);

	const char *node_name = tok_to_str(node->data);

	if (STREQ(node_name, "FUNCTION")) return CST_NODE_FUNC;
	else if (STREQ(node_name, "STMT")) return CST_NODE_STMT;
	// ...

	errno = SLANG_BAD_PARSE_TREE;
	raise_error("parser_defs.c", slang_create_error_token(0, 0), NULL);
	return CST_NODE_INVALID;
}