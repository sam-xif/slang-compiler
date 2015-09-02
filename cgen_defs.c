
#include "includes.h"


void _convert_stmt_block(st_node_t *root, stmt_list_t *out_list) {
}


program_def_t *cgen_parse_tree_to_program_def(st_node_t *root) {
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

	// Allocate return value.
	program_def_t *ret = MALLOC_ONE(program_def_t);

	int i;
	for (i = 0; i < functions->length; i++) {
		function_def_t *function = MALLOC_ONE(function_def_t);
		function->name = tok_to_str((*GET_ITEM_AS(GET_ITEM_AS(functions, i, st_node_t)->children, 2, st_node_t *))->data);
		st_node_t *body_node = st_get_first_by_name(GET_ITEM_AS(functions, i, st_node_t), "BLOCK");
		_convert_stmt_block(body_node, function->body);
	}


	// Clean up.
	list_free(aliases, NULL);
	list_free(file_ns, NULL);
	list_free(functions, NULL);
	list_free(tags, NULL);
	list_free(references, NULL);
	list_free(use_directives, NULL);
	list_free(func_defs, NULL);
}