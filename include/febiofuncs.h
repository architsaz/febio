#ifndef FEBIO_H
#define FEBIO_H

#include "mystructs.h"

int rinputf(char *, mesh *, input *);
int mkdirs(void);
int datafiles(void);
int read_zfem(char *, int *, int *, double **, int **);
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
int readfebiolog(char *, mesh *, double **, read_time);
int sortedsv(mesh *, double *, double *, double **, double **);
int unibimask(mesh *, double *, double *, int **);
void mystat(double *, int, double *, double **);
int analzs(mesh *, double *, double *, char *, char *);
int redanals_msa1(mesh *, int *, double *, char *);
int find_crit_anglvec(mesh *, double *, double **, int **);
#endif // FEBIO_H
