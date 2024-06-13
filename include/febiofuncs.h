#ifndef FEBIO_H
#define FEBIO_H

int rinputf(mesh *,input *);
int mkdirs();
int datafiles();
char *edit_endline_character(char *, int , FILE *);
int read_zfem(char *,int *, int *, double **,int **);
int read_wallmask(char *,mesh *,input *,int **);
int read_regionmask(char *,mesh *,input *,int **, int **);

#endif // FEBIO_H
