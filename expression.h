

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

typedef enum {
	EXPR_ARITH = 1,
	EXPR_FUNC_CALL = 2,
	EXPR_BOOL = 3
} expr_kind;

// An expression
typedef struct {
	expr_kind type;
} expr_t;

#endif // _EXPRESSION_H
