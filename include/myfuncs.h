#ifndef MYFUNCS_H
#define MYFUNCS_H
// Define a function pointer type for comparison functions
    int checkEIDS(int *);
    int ConverMesh(mesh *, mesh *,ConvertorFunc );
    void tri3_to_tri6(mesh *, mesh **);
    void tri3_to_quad4(mesh *, mesh **);
    void SCA_int_VTK(FILE *,char *,int ,int ,void *);
    void SCA_double_VTK(FILE *,char *,int ,int ,void *);
    int SaveVTK(char *, char *,int ,mesh *,elemVTK ,FunctionWithArgs *,size_t,FunctionWithArgs *,size_t );
    void tri3funcVTK(FILE *,int ,int *);
    void tri6funcVTK(FILE *,int ,int *);
#endif // MYFUNCS_H

