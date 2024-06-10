#ifndef FEBIO_H
#define FEBIO_H

int rinputf();
int mkdirs();
int datafiles();
char *edit_endline_character(char *line, int buffer, FILE *fptr);
int read_zfem(char *path,int *npoin, int *nelem, double **ptxyz,int **elems);
int read_wallmask(char *path,int **Melem2);
int read_regionmask(char *path, int **region_id2, int **region_idp2);


#endif // FEBIO_H
