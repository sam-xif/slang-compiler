/*
 * Defines a type
 */ 

#ifndef _SLANG_TYPE_H
#define _SLANG_TYPE_H

enum type_kind {
	BASE = 1,
	AGGREGATE
};


typedef struct type_def {
	const char *name;
	enum type_kind type;
	union {
		struct {
			int size;
			bool _signed;
		} base_data;

		struct {
			struct declaration *decl_arr;
			int num_fields;
		} aggregate_data;
	} type_data;
} type_def_t;


int get_size(type_def_t *type);

#endif // _SLANG_TYPE_H