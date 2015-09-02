/*
 * Defines utilities for generating intermediate language code to be passed to an optimizer or assembly code generator.
 */


#ifndef _INTERMEDIATE_CGEN_H
#define _INTERMEDIATE_CGEN_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ilcgen_state {
	int temp_var_index;
	int label_index;
} ilcgen_state_t;


#ifdef __cplusplus
}
#endif

#endif // _INTERMEDIATE_CGEN_H