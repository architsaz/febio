#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

// Lists of macros
#define PI 3.14159
#define dimension 3 // dimension of domain
#define EPSILON 1e-9
#define MAX_ITER 100 // Maximum number of iterations

#define SQUARE(x) ((x) * (x))
#define ABS(x) ((x > 0) ? (x) : -(x))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define IDX(row, col, n) ((row) * (n) + (col))
// Free each pointer and set to NULL, with debug messages for clarity
#define SAFE_FREE(ptr)  \
	do                  \
	{                   \
		if (ptr)        \
		{               \
			free(ptr);  \
			ptr = NULL; \
		}               \
	} while (0)

// Macro to check function return value and print an error message
#define CHECK_ERROR(func)                                                         \
	do                                                                            \
	{                                                                             \
		int ret = (func);                                                         \
		if (ret != 0)                                                             \
		{                                                                         \
			fprintf(stderr, "Error: %s failed with error code %d\n", #func, ret); \
			exit(ret);                                                            \
		}                                                                         \
	} while (0)

#endif // COMMON_H
