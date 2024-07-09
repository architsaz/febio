#ifndef FEBIO_H
#define FEBIO_H

int rinputf(mesh *,input *);
int mkdirs();
int datafiles();
char *edit_endline_character(char *, int , FILE *);
int read_zfem(char *,int *, int *, double **,int **);
int read_wallmask(char *,mesh *,input *,int **);
int read_regionmask(char *,mesh *,input *,int **, int **);
int calctrithick (mesh *, input *);
int calctriyoung (mesh *, input *);
int calctripres(mesh *, input *);
int calctrifixb(mesh *, input *);
int febmkr(char *, char *,int ,mesh *,input *);
int runfebio();
int readNJ();
int checkresult();
int appliedgfilt_ptri6(mesh *,double *,int );
int appliedgfilt_etri(mesh *,double *, int );
#endif // FEBIO_H
