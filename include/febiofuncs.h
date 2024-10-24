#ifndef FEBIO_H
#define FEBIO_H

#include "mystructs.h"

int rinputf(char *, mesh *, input *);
int mkdirs(void);
int datafiles(void);
int read_zfem(char *, int *, int *, double **, int **);
int readgz_zfem(char *, int *, int *, double **, int **);
int read_wallmask(char *, mesh *, input *, int **);
int read_regionmask(char *, mesh *, input *, int **, int **);
int read_BCmask(char *, mesh *, int **);
int calctrithick(mesh *, input *);
int calctriyoung(mesh *, input *);
int calctripres(mesh *, mesh *, input *, int *);
int calctrifixb(mesh *, mesh *, input *, int *);
int mkdomain(int, int *, int *, input *, int **);
int cleanBCmasks(mesh *, int *, int *, int **);
void NeoHokfunc(FILE *, input *);
void Elementfunc(FILE *, mesh *);
void shellthickfunc(FILE *, mesh *);
void pres_stepfunc(FILE *);
void norm_stepfunc(FILE *);
int febmkr(char *, char *, int, mesh *, input *);
int runfebio(int);
int readNJ(int);
runstatus checkresult(char *);
int appliedgfilt_ptri6(mesh *, double *, int);
int appliedgfilt_etri(mesh *, double *, int);
int readfebiolog(char *path, mesh *M, double **st2, read_time logtime);
int sortedsv(mesh *M, double *eigenvalue, double *eigenvector, double **sorted_s2, double **sorted_v2);
int unibimask(mesh *M, double *smax1, double *smax2, int **sdir2);
void mystat(double *arr, int n, double *area, double **output1);
int analzs(mesh *M, double *area, double *smax1, char *casename, char *study);
int redanals_msa1(mesh *M, int *sdir, double *area, char *casename);
#endif // FEBIO_H
