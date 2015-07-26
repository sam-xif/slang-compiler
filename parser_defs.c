
#include "includes.h"

static void _free_pt_func(pt_node_t *node) {
	if (node == NULL) return;

	// Possibly replace with checking the node_type instead, as its more reliable
	if (node->type != NODE_LEAF) {
		int i;
		for (i = 0; i < node->children->length; i++) {
			// Recurse

			// A pointer to a pointer to a node
			pt_node_t **child = GET_ITEM_AS(node->children, i, pt_node_t *);

			if ((*child)->type != NODE_LEAF) {
				_free_pt_func(*child);
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

void parser_free_pt(pt_node_t *root, bool free_root) {
	if (root == NULL) return;

	if (root->type != NODE_LEAF) {
		_free_pt_func(root);
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

	parser_free_pt(p->result, true);
	list_free(p->token_list, NULL);
	free(p);
}

static bool _get_leaves_selector(pt_node_t *node, bool *usable) {
	if (node->type == NODE_LEAF) return true;
	else return false;
}

node_list_t *pt_get_leaves(pt_node_t *node) {
	if (node->type == NODE_LEAF) {
		puts("Unable to get the leaves of a node with no children.");
		errno = EINVAL;
		return NULL;
	}

	return pt_select_nodes(node, &_get_leaves_selector);
}



// Implemented mainly for debugging purposes now, but will be useful later.
// The selector function pointer can be NULL if one wants every single node of the tree to be returned.
node_list_t *pt_select_nodes(pt_node_t *root, bool(*selector_func)(pt_node_t *, bool *)) {
	assert(root != NULL);

	node_list_t *ret = (node_list_t *)LIST_CREATE(pt_node_t);
	bool usable = true;

	// First check the root node:
	if (selector_func == NULL) list_add_item(root, ret);
	else if (selector_func(root, &usable) == true) list_add_item(root, ret);

	if (usable == true) {
		if (root->type != NODE_LEAF && root->children != NULL) {
			int i;
			for (i = 0; i < root->children->length; i++) {
				// We cast to pt_node_t* because pointers to pointers to nodes are stored in the children arrays.
				// This is also why we dereference once, so that we can obtain the pointer to the data, not the pointer to the pointer.
				node_list_t *result = pt_select_nodes(*(GET_ITEM_AS(root->children, i, pt_node_t *)), selector_func);
				list_concatenate(ret, result);
				list_free(result, NULL);
			}
		}
	}

	return ret;
}


node_list_t *pt_parameterized_select_nodes(pt_node_t *root, void *data, bool(*selector_func)(pt_node_t *, bool *, void *)) {
	assert(root != NULL);

	node_list_t *ret = (node_list_t *)LIST_CREATE(pt_node_t);
	bool usable = true;

	// First check the root node:
	if (selector_func == NULL) list_add_item(root, ret);
	else if (selector_func(root, &usable, data) == true) list_add_item(root, ret);

	if (usable == true) {
		if (root->type != NODE_LEAF && root->children != NULL) {
			int i;
			for (i = 0; i < root->children->length; i++) {
				// We cast to pt_node_t* because pointers to pointers to nodes are stored in the children arrays.
				// This is also why we dereference once, so that we can obtain the pointer to the data, not the pointer to the pointer.
				node_list_t *result = pt_parameterized_select_nodes(*(GET_ITEM_AS(root->children, i, pt_node_t *)), 
					data, selector_func);
				list_concatenate(ret, result);
				list_free(result, NULL);
			}
		}
	}

	return ret;
}


static bool _get_nodes_by_name_selector(pt_node_t *node, bool *usable, void *data) {
	const char *name = (const char *)data;
	const char *node_name = tok_to_str(node->data);
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
	else return false;
}


node_list_t *pt_get_nodes_by_name(pt_node_t *root, const char *name) {
	return pt_parameterized_select_nodes(root, (void *)name, &_get_nodes_by_name_selector);
}
