/*
 * Global parser definitions
 */

#ifndef _PARSER_DEFS_H
#define _PARSER_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#define NODE_LIST_DEFAULT_CAPACITY 5


// TODO: Possibly create typedefs of list_t for each type for easier readability? Like so:
typedef list_t node_list_t;

typedef enum pt_node_type {
	NODE_ROOT = 0,
	NODE_NT = 1,			// Non-terminal node
	NODE_LEAF = 2
} pt_node_type;


// TODO: Add a child_index property to the pt_node structure

// Less important structs are typedefed from the start I guess...
typedef struct pt_node {
	pt_node_type type;

	token_t data;

	/* The following void pointers must be of type pt_node, and should be casted to pt_node for access. */
	node_list_t *children;		/* A list of children, from left to right. NULL if leaf node.
								* This list is of type pt_node_t*. Pointers are to be stored in the array
								*/
	struct pt_node *parent;		// NULL if root node
	int child_index;			/* The index of this node in its parent's child array */
} pt_node_t; // Parse tree node.

typedef enum parser_state {
	PARSE_ERR = 0,
	PARSE_START,
	PARSE_WORKING,
	PARSE_FINISH,
} parse_state_t;

typedef int parse_err_t;

typedef struct parser {
	list_t *token_list;
	int index;
	parse_state_t state;
	parse_err_t error;
	pt_node_t *result;
	stack_t *symbol_stack;

	cfg_t *grammar;	/* The grammar currently in use */

	// For storing temporary values
	int save_index;
	bool save_result;
} parser_t; // Important types such as parser and scanner get the slang_ prefix

// Removes entire parse tree from memory
void parser_free_pt(pt_node_t *root, bool free_root);

parser_t *parser_alloc(cfg_t *grammar);
void parser_free(parser_t *p, bool free_cfg);


/* === Parse Tree manipulation utility functions === */

/* 
 * Traverses the parse tree from left to right, passing each node into a selector function 
 * that decides whether or not to include that node in the final output list.
 *
 * The bool pointer in the selector func parameter list is to indicate whether or not the node is usable after
 * passing it to the selector function. This should be set false only if an error will occur by attempting to
 * access the child nodes. This occurs mainly when the node is released from memory.
 */
node_list_t *pt_select_nodes(pt_node_t *root, bool(*selector_func)(pt_node_t *, bool *));

// The selector function now takes a void pointer that points to a piece of user data.
// The selector function must know what to cast this data to, or it will be unusable.
node_list_t *pt_parameterized_select_nodes(pt_node_t *root, void *data, bool(*selector_func)(pt_node_t *, bool *, void *));

node_list_t *pt_get_nodes_by_name(pt_node_t *root, const char *name);

// Gets the leaf nodes from a node (from left to right).
node_list_t *pt_get_leaves(pt_node_t *node);
node_list_t *pt_get_siblings(pt_node_t *node);
pt_node_t *pt_get_child(pt_node_t *parent, int index);

#ifdef __cplusplus
}
#endif

#endif // _PARSER_DEFS_H