#ifndef GLOBALPARAFEB_H
#define GLOBALPARAFEB_H

#include <stdio.h>
#include <stdlib.h>


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

	extern int NrNj;
	extern double *NJmask;
// loading curve 
	extern int np_lc;
	extern double *time_lc,*load_lc;	

#endif // GLOBALPARAFEB
