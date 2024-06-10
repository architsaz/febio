#ifndef FEBIO_H
#define FEBIO_H

int rinputf();
int mkdirs();
int datafiles();
int read_zfem(char *path,int *npoin, int *nelem, double **ptxyz,int **elems);
char *edit_endline_character(char *line, int buffer, FILE *fptr);



#endif // FEBIO_H
