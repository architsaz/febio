#ifndef FEBIO_H
#define FEBIO_H

#include "mystructs.h"

int rinputf(char *,mesh *,input *);
int mkdirs(int );
int datafiles(void);
int read_zfem(char *,int *, int *, double **,int **);
int readgz_zfem(char *,int *, int *, double **,int **);
int read_wallmask(char *,mesh *,input *,int **);
int read_regionmask(char *,mesh *,input *,int **, int **);
int calctrithick (mesh *, input *);
int calctriyoung (mesh *, input *);
int calctripres(mesh *, input *);
int calctrifixb(mesh *, input *);
void NeoHokfunc(FILE *, input *);
void Elementfunc(FILE *, mesh *);
void shellthickfunc(FILE *, mesh *);
void pres_stepfunc(FILE *);
void norm_stepfunc(FILE *);
int febmkr(char *, char *,int ,mesh *,input *);
int runfebio(int );
int readNJ(int );
runstatus checkresult(char *);
int appliedgfilt_ptri6(mesh *,double *,int );
int appliedgfilt_etri(mesh *,double *, int );
#endif // FEBIO_H
