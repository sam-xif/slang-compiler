/*
* Provides large data structures for storing data that is read by the parser
*/

#ifndef _OBJECT_H
#define _OBJECT_H

typedef enum {
	OBJ_FUNC = 1,
	OBJ_CLASS = 2,
	OBJ_VAR = 3,
	OBJ_BLOCK = 4
} obj_kind;

typedef struct {
	obj_kind type;

	union data {
		struct func_t {
			char *func_name;
			block_t *func_block;
		};

		struct var_t {
			char *var_name;
			// Define the variable's type
		};
	};
} data_obj_t;

typedef data_obj_t SL_object;

#endif // _OBJECT_H