
#include "includes.h"


// Checks if a symbol is a terminal of cfg
bool cfg_is_terminal(token_t *symbol, cfg_t *cfg) {
	return list_contains_item(symbol, cfg->terminals, &cfg_symbol_compare_strict);
}

// Checks if a symbol is a non-terminal of cfg
bool cfg_is_non_terminal(token_t *symbol, cfg_t *cfg) {
	return list_contains_item(symbol, cfg->non_terminals, &cfg_symbol_compare_strict);
}

bool cfg_is_start_symbol(token_t *symbol, cfg_t *cfg) {
	return list_contains_item(symbol, cfg->start_symbols, &cfg_symbol_compare_strict);
}

bool cfg_symbol_compare(token_t *t1, token_t *t2) {
	// Now it is changed so that if there is no data_ptr for one token, then it only matches the data types.
	// This is the behavior that was intended.
	if (t1->data_ptr == NULL || t2->data_ptr == NULL) {
		return t1->type == t2->type;
	}
	
	//else if (t1->data_ptr == NULL || t2->data_ptr == NULL) {
		// only one is null, 
	//	return false;
	//}
	else {
		char *str1 = tok_to_str(*t1);
		char *str2 = tok_to_str(*t2);

		bool str_cmp = STREQ(str1, str2);

		free(str1);
		free(str2);

		return str_cmp;
	}
}

bool cfg_symbol_compare_strict(token_t *t1, token_t *t2) {
	// Now it is changed so that if there is no data_ptr for one token, then it only matches the data types.
	// This is the behavior that was intended.
	if (t1->data_ptr == NULL && t2->data_ptr == NULL) {
		return t1->type == t2->type;
	}

	else if (t1->data_ptr == NULL || t2->data_ptr == NULL) {
		// only one is null
		return false;
	}
	else {
		char *str1 = tok_to_str(*t1);
		char *str2 = tok_to_str(*t2);

		bool str_cmp = STREQ(str1, str2);

		free(str1);
		free(str2);

		return str_cmp;
	}
}

cfg_rule_t *cfg_get_rule_by_name(const char *name, cfg_t *cfg) {
	int i;
	for (i = 0; i < cfg->rule_list->length; i++) {
		const char *cur_symbol = tok_to_str(*(GET_ITEM_AS(cfg->rule_list, i, cfg_rule_t)->start_symbol));

		if (STREQ(cur_symbol, name)) {
			free(cur_symbol);
			break;
		}
		else {
			free(cur_symbol);
		}
	}

	return GET_ITEM_AS(cfg->rule_list, i, cfg_rule_t);
}


cfg_t *cfg_alloc() {
	cfg_t *ret = MALLOC_ONE(cfg_t);
	int capacity = 5;
	ret->non_terminals = list_alloc(capacity, sizeof(token_t));
	ret->terminals = list_alloc(capacity, sizeof(token_t));
	ret->start_symbols = list_alloc(capacity, sizeof(token_t));
	ret->rule_list = list_alloc(capacity, sizeof(cfg_rule_t));
	return ret;
}


cfg_rule_t *cfg_rule_alloc() {
	cfg_rule_t *ret = MALLOC_ONE(cfg_rule_t);
	ret->match_data = list_alloc(5, sizeof(match_list_t));
	ret->start_symbol = MALLOC_ONE(token_t);
	return ret;
}


// Do NOT call cfg_rule_free on the rules of this cfg before calling cfg_free, as this function will do it itself.
void cfg_free(cfg_t *cfg) {
	if (cfg == NULL) return;

	list_free(cfg->rule_list, &cfg_rule_free);
	list_free(cfg->start_symbols, NULL);
	list_free(cfg->non_terminals, NULL);
	list_free(cfg->terminals, NULL);
	free(cfg);
}

static void _match_free_func(match_t *match) {
	free(match->error_msg);
}

static void _rule_match_data_free_func(match_list_t *list) {
	list_free(list->matches, &_match_free_func);
}


void cfg_rule_free(cfg_rule_t *rule) {
	if (rule == NULL) return;

	list_free(rule->match_data, &_rule_match_data_free_func);
	free(rule->start_symbol);
	//free(rule);
}