/*
 * Global includes that are to be included in every source file
 */

#ifndef _INCLUDES_H
#define _INCLUDES_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#ifdef _WINDOWS
#	include <Windows.h>
#else
// Add Linux/Mac headers here
#endif

#include <stdbool.h>
#include <assert.h>

#include "slang.h"

#endif /* _INCLUDES_H */