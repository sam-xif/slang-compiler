/*
 * Implementation of an LL parser
 */

#ifndef _PARSER_LL_H
#define _PARSER_LL_H

#ifdef __cplusplus
extern "C" {
#endif

struct ll_parse_table {
	token_list_t *rows;
	token_list_t *columns;
	list_t *data;
};

// TEMPORARY
pt_node_t *parse_stack(token_list_t *tok_list, parser_t *parse);

#ifdef __cplusplus
}
#endif


#endif // _PARSER_LL_H