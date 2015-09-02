/*
 * Static type-checker
 */

#ifndef _TYPECHECK_H
#define _TYPECHECK_H

/* Temporary */
struct declaration {
	const char *name;
	struct type_def *type;
};

#endif // _TYPECHECK_H