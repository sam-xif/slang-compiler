/*
 * Defines utilities for parsing token collections using the recursive descent algorithm.
 */


#ifndef _PARSER_RD_H
#define _PARSER_RD_H

#ifdef __cplusplus
extern "C" {
#endif


// Match using the recursive descent algorithm.
pt_node_t *parse_recursive_descent(token_list_t *tok_list, parser_t *parse);

// Rewrites the tree to make it more organized and logical.
// Returns the pointer to the node that was passed into the function.
pt_node_t *parse_rd_rewrite_tree(pt_node_t *root);

#ifdef __cplusplus
}
#endif

#endif // _PARSER_RD_H