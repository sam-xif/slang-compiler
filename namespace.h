/*
 * This file may not be used...
 * Defines a namespace where identifiers such as type and function names are stored
 */

#ifndef _NAMESPACE_H
#define _NAMESPACE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct namespace_def {
	const char *name;
	list_t *types;
	list_t *functions;
	list_t *global_vars;
	list_t *structs;
} namespace_def_t;


namespace_def_t *ns_build_from_program(struct program_def *program);
namespace_def_t *ns_init_global_namespace();

void ns_add_alias(struct alias_def *a, namespace_def_t *ns);
void ns_write_to_stream(namespace_def_t *ns, FILE *fp);


#ifdef __cplusplus
}
#endif

#endif // _NAMESPACE_H