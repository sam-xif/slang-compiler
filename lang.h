/* 
 * Header file that defines various functions and structs for reading the 
 * lang.txt file that defines the programming language to be compiled.
 */

#ifndef _LANG_H
#define _LANG_H


#define SCANNER_TAG ("SCANNER")
#define PARSER_TAG ("PARSER")
#define CFG_TAG ("CFG")
#define ILCGEN_TAG ("ILCGEN")
#define CGEN_TAG ("CGEN")
#define MACRO_REPLACE ("replace")

// put back the ifdef __cplusplus later

#ifdef __cplusplus
extern "C" {
#endif

typedef enum var_type {
	LANG_STR_CONST = 1,
	LANG_STR_ARRAY = 2
} var_type;

typedef struct lang_var {
	var_type type;
	char **value; // Pointer to the value
	unsigned int length; // Length of the array if value points to an array
} lang_var_t;


// Tags are required to be in the order: SCANNER, PARSER, CFG, ILCGEN, CGEN
typedef struct lang_state {
	list_t *tok_list;

	int scanner_index;
	int parser_index;
	int cfg_index;
	int ilcgen_index;
	int cgen_index;
} lang_state_t;

// Parses a buffer that is conformant to the lang.txt format.
lang_state_t *lang_txt_parse_buffer(const char *buf, int length);

list_t *lang_txt_preprocess(const char *buf, int length); // to be removed once the function can be made static.
cfg_t *lang_get_cfg(lang_state_t *state);
lang_var_t *lang_get_var(char *name);

void lang_free(lang_state_t *state);


#ifdef __cplusplus
}
#endif


#endif // _LANG_H
