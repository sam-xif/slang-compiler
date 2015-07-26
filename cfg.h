/*
 * Header that defines utilities for using context-free grammars
 */

#ifndef _GRAMMAR_H
#define _GRAMMAR_H

typedef struct cfg_rule {
	token_t *start_symbol;	/* Change to value */
	list_t *match_data;		/* Two dimensional array of match_t structures.
							 * The match_data list holds list_t's in its array, which in turn hold match_t's
							 */
	bool throw_error;
	int error_code;
} cfg_rule_t;

typedef struct cfg {
	list_t *start_symbols;
	list_t *terminals;
	list_t *non_terminals;

	list_t *rule_list;
} cfg_t;

// Checks if a symbol is a terminal of cfg
bool cfg_is_terminal(token_t *symbol, cfg_t *cfg);

// Checks if a symbol is a non-terminal of cfg
bool cfg_is_non_terminal(token_t *symbol, cfg_t *cfg);

bool cfg_is_start_symbol(token_t *symbol, cfg_t *cfg);

cfg_rule_t *cfg_get_rule_by_name(const char *name, cfg_t *cfg);

// Allocates and initializes a new cfg object
cfg_t *cfg_alloc();
// Allocates and initializes a new cfg_rule object
cfg_rule_t *cfg_rule_alloc();

// Clean up functions
void cfg_free(cfg_t *cfg);
void cfg_rule_free(cfg_rule_t *rule);

/*
 * The parameters need to be pointers in order to conform to the function pointer definition in list.
 * This is because there is no way to dereference a void pointer, so the pointer is passed to the function, and dereferenced there.
 *
 * OPTIONAL: In the future, if all token_t parameters will be changed to values/pointers, create a wrapper function that satisfies
 * the function pointer definition. For example: 
 *
 *		static bool _symbol_compare(token_t *t1, token_t *t2) {
 *			cfg_symbol_compare(*t1, *t2);
 *		}
 */
bool cfg_symbol_compare(token_t *t1, token_t *t2);
bool cfg_symbol_compare_strict(token_t *t1, token_t *t2);


#endif // _GRAMMAR_H
