#ifndef GLOBALPARAPPA_H
#define GLOBALPARAPPA_H

#include <stdio.h>
#include <stdlib.h>

// declare global parameters
// directories:
extern char pst_rundir[50];
extern char pst_rundir2[50];
extern char pstdir[50];
extern char pst_datadir[50];
// data file
extern char past_datafilepath[10][500];
// past_filename
extern char febname[10];
extern char past_filename[50];
extern char study[50];
extern char study2[50];
extern char readingtime1[50];
extern char readingtime2[50];
extern char iteration[50];
extern char iteration2[50];
extern int num_study;
extern int num_iteration;
extern int num_time;

#endif // GLOBALPARAFEB
