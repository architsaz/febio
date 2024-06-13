#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

// Lists of macros
	#define PI 3.14159
	#define dimension  3 // dimension of domain 

	#define SQUARE(x) ((x) * (x))
	#define ABS(x) ((x>0) ? (x) : -(x))
	#define MAX(x, y) (((x) > (y)) ? (x) : (y))
	#define MIN(x, y) (((x) < (y)) ? (x) : (y))
// Macro to check function return value and print an error message
	#define CHECK_ERROR(func) do { \
	    int ret = (func); \
	    if (ret != 0) { \
	        fprintf(stderr, "Error: %s failed with error code %d\n", #func, ret); \
	        exit(ret); \
	    } \
	} while (0)
	
// directories:
	extern    char rundir[50];
    extern	char datadir[50];
    extern	char slash[10];
	extern char dot[10];

// Solver 
	extern	char nonlinear_FE[50];
	extern	int symetric_stiff;

	//data file 
	extern char datafilepath [10][500];
	extern char filename[50]; 

#endif // COMMON_H
