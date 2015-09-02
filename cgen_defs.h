/*
 * Defines structures and functions for use by the code generator.
 */

#ifndef _CGEN_DEFS_H
#define _CGEN_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif


/* Enums */

typedef enum stmt_kind {
	STMT_ASSIGN = 1,
	STMT_IF,
	STMT_IF_ELSE,
	STMT_WHILE
} stmt_kind;


enum alias_kind {
	ALIAS_FUNCTION = 1,
	ALIAS_IDENT,
	ALIAS_TYPE,
	ALIAS_STRUCT
};

/* 
 * Defined this way so that different values can be bitwise OR'd together.
 * Example: HIDDEN | EXTERNAL. 
 * This produces a value 0000 0101, and so it is easy to see both enum values included there.
 * HIDDEN and GLOBAL conflict with each other, however, and the compiler will throw an error if they are used together.
 */
enum access_level {
	HIDDEN = 1,
	GLOBAL = 2,
	EXTERNAL = 4
};


/* Data structures */

struct if_else_stmt {
	stmt_kind predicate;
};


struct basic_type {
	const char *name;
	int size;
	bool _signed;
};


typedef struct stmt_def {
	stmt_kind type;
	union stmt_data {
		struct if_else_stmt *if_else;
	};
} stmt_def_t;


typedef list_t stmt_list_t;
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
	enum access_level access;
	stmt_list_t *body;	/* Optional function body. Can be NULL */
} function_def_t;


typedef struct alias_def {
	const char *source_name;
	const char *target_name;
	enum alias_kind type;
} alias_def_t;


typedef struct program_def {
	const char *namespace_directive;
	function_list_t *function_list;
} program_def_t;



program_def_t *cgen_parse_tree_to_program_def(st_node_t *root);


#ifdef __cplusplus
}
#endif

#endif // _CGEN_DEFS_H