/*
 * Defines structures and functions for use by the code generator.
 */

#ifndef _CGEN_DEFS_H
#define _CGEN_DEFS_H
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

/* Enums */

typedef enum stmt_kind {
	STMT_ASSIGN = 1,
	STMT_IF = 2,
	STMT_IF_ELSE = 3,
	STMT_WHILE = 4
} stmt_kind;

/* Data structures */

struct if_else_stmt {
	stmt_kind predicate;
};

typedef struct stmt_def {
	stmt_kind type;
	union stmt_data {
		struct if_else_stmt *if_else;
	};
} stmt_def_t;

typedef list_t function_list_t;

typedef struct parameter_def {
	const char *type;
	const char *name;
} parameter_def_t;

typedef list_t parameter_list_t;

typedef struct function_def {
	parameter_list_t *parameters;
	const char *name;
	const char *return_type;
} function_def_t;

typedef struct program_def {
	function_list_t *function_list;
} program_def_t;


program_def_t *cgen_parse_tree_to_program_def(pt_node_t *tree_root);

/*
#ifdef __cplusplus
}
#endif
*/
#endif // _CGEN_DEFS_H