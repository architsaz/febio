#ifndef MYFUNCS_H
#define MYFUNCS_H
// Define a function pointer type for comparison functions
    char *edit_endline_character(char *, int , FILE *);
    int checkEIDS(int *);
    int ConverMesh(mesh *, mesh *,ConvertorFunc );
    void tri3_to_tri6(mesh *, mesh **);
    void tri3_to_quad4(mesh *, mesh **);
    void SCA_int_VTK(FILE *,char *,int ,int ,void *);
    void SCA_double_VTK(FILE *,char *,int ,int ,void *);
    void read_VTK_double(FILE *fptr,int col,int nr,void **field);
    void read_VTK_int(FILE *fptr,int col,int nr,void **field);
    int SaveVTK(char *, char *,int ,mesh *,elemVTK ,FunctionWithArgs *,size_t,FunctionWithArgs *,size_t );
    int ReadVTK(char *, char *,int ,FunctionWithArgs2 *,int );
    void tri3funcVTK(FILE *,int ,int *);
    void tri6funcVTK(FILE *,int ,int *);
    int countline(char *);
#endif // MYFUNCS_H

