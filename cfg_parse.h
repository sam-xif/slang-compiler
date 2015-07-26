/*
 * Provides functions and structures for parsing cfgs in the format that they are found in the language file.
 */

#ifndef _CFG_PARSE_H
#define _CFG_PARSE_H

// TODO: Add in ifdef __cplusplus

#define CFG_RULE_SEPARATOR ('|')
#define CFG_RULE_TERMINATOR (';')
#define CFG_ERROR_TAG ("ERROR")

#define GET_MATCH(x, y, cfg_rule) ((match_t *)(((list_t *)(cfg_rule->match_data->array))[x].array))[y]

typedef struct match {
	token_t match_token; /* If data_ptr is NULL, then only token_type is matched */

	bool is_terminal;

	int error_code;
	const char *error_msg;
} match_t;

typedef struct {
	list_t *matches;
	bool is_terminal;
} match_list_t;

typedef struct match_list_arr {
	match_list_t *match_list_array;
	int length;
} match_list_arr_t;

int cfg_parse_one(list_t *tok_list, int start_index, cfg_t *cfg);

match_list_t *match_list_alloc();
void match_list_free(match_list_t *list);


#endif // _CFG_PARSE_H