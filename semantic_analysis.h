/*
 * Provides functions for confirming the validity of a parse tree.
 */

#ifndef _SEMANTIC_ANALYSIS_H
#define _SEMANTIC_ANALYSIS_H

typedef struct context_state {
	list_t *declared_idents;
	list_t *defined_functions;
	/* Other context info here */
} context_state_t;

struct symbol_table_ent {
	const char *name;
};


typedef list_t table_ent_list_t;


typedef struct symbol_table {
	table_ent_list_t *entry_list;

	struct symbol_table *parent_scope;
	list_t *children_scopes;	/* A list of type symbol_table */
} symbol_table_t;


/* Node with support for semantic attributes */
typedef struct semantic_node {
	st_node_t *base_node;

	type_def_t *type;
	void *value;
} semantic_node_t;


semantic_node_t *semant_convert_pt(st_node_t *root);

#endif // _SEMANTIC_ANALYSIS_H