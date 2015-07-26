
#ifndef _GLOBAL_DEFS_H
#define _GLOBAL_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

	// String comparison macros
#define STREQ(a,b) (strcmp(a,b) == 0)
#define STREQ_N(a,b,n) (strncmp(a,b,n) == 0)


	// Possibly even remove these, and create a slang_perror function
#ifndef NOERROR
#	define NOERROR 0
#endif
#ifndef EINVAL
#	define EINVAL 22
#endif
#ifndef ENOENT
#	define ENOENT 2
#endif


#undef STRCAT
#undef STRNCAT
#undef STRCPY
#undef STRNCPY
#undef MEMCPY
#undef MEMMOVE

	// TODO: Implement these macros in the source:
	/*
	 * These macros are to (hopefully) resolve issues with platform dependency
	 * dst is the destination buffer, dst_size is the size of the buffer,
	 * src is the source buffer, and cnt is the maximum number of bytes to copy
	 */
#ifdef _WINDOWS
#	define STRCAT(dst, dst_size, src) strcat_s(dst, dst_size, src)
#	define STRNCAT(dst, dst_size, src, cnt) strncat_s(dst, dst_size, src, cnt)
#	define STRCPY(dst, dst_size, src) strcpy_s(dst, dst_size, src)
#	define STRNCPY(dst, dst_size, src, cnt) strncpy_s(dst, dst_size, src, cnt)
#	define MEMCPY(dst, dst_size, src, cnt) memcpy_s(dst, dst_size, src, cnt)
#	define MEMMOVE(dst, dst_size, src, cnt) memmove_s(dst, dst_size, src, cnt)
#else
#	define STRCAT(dst, dst_size, src) assert(dst_size - strlen(dst) > strlen(src)); strcat(dst, src)
#	define STRNCAT(dst, dst_size, src, cnt) assert(dst_size - strlen(dst) > strlen(src)); strncat(dst, src, cnt)
#	define STRCPY(dst, dst_size, src) assert(dst_size - strlen(dst) > strlen(src)); strcpy(dst, dst_size, src)
#	define STRNCPY(dst, dst_size, src, cnt) assert(dst_size - strlen(dst) > cnt); strncpy(dst, src, cnt)
#	define MEMCPY(dst, dst_size, src, cnt) assert(dst_size >= cnt); memcpy(dst, src, cnt)
#	define MEMMOVE(dst, dst_size, src, cnt) assert(dst_size >= cnt); memmove(dst, src, cnt)
#endif

	/*
	 * Because of the way these macros are made, you can still assign the return value of func to a variable. For example:
	 * a = RETURN_IF_ERROR( myFunc() );
	 *
	 * When expanded, the code is:
	 * a = myFunc();
	 * if (errno != NOERROR) {
	 *		perror("Error");
	 *		return;
	 * }
	 *
	 * The variable a gets the return value of myFunc(). The same applies for RETURN_VAL_IF_ERROR
	 */

	/*
	 * Prints an error message and returns from the function if, after executing func,
	 * which could be a function or an expression, errno is set to a value.
	 * This macro is pointless if the function func does not set errno, and if the function doesn't set errno,
	 * then custom code should be written for that case.
	 */
#define RETURN_IF_ERROR(func) func; if (errno != NOERROR) { raise_error(NULL, slang_create_error_token(0, 0), NULL); return; }

	/*
	 * Returns from the function like the above macro, but returns the value specified by val_err
	 * (for functions that don't return void).
	 */
#define RETURN_VAL_IF_ERROR(func, val_err) func; if (errno != NOERROR) { raise_error(NULL, slang_create_error_token(0, 0), NULL); return val_err; }



	// In case any of these are defined for some reason
#undef MALLOC
#undef CALLOC
#undef MALLOC_ONE

	// Allocates space for n objects.
#define MALLOC(n, t) (t *)malloc(n * sizeof(t))
#define CALLOC(n, t) (t *)calloc(n, sizeof(t))

	// Allocates space for one object.
#define MALLOC_ONE(t) (t *)malloc(sizeof(t))

	// Macro to free a pointer then nullify it.
#define FREE_PTR(p) free(p); p = NULL


	// Macro to allocate memory for a string. Sets ret to the resulting pointer.
#define MALLOC_STRING(str, ret) char *__str = str;\
	char *__buf = MALLOC(strlen(str) + 1, char);\
	STRCPY(__buf, strlen(str) + 1, __str);\
	ret = __buf


/* Typedefs */

typedef unsigned int uint;

#ifdef __cplusplus
}
#endif


#endif // _GLOBAL_DEFS_H