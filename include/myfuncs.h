#ifndef MYFUNCS_H
#define MYFUNCS_H
// Define a function pointer type for comparison functions
    int checkEIDS(int *);
    int ConverMesh(mesh *, mesh *,ConvertorFunc );
    void tri3_to_tri6(mesh *, mesh **);
    void tri3_to_quad4(mesh *, mesh **);
#endif // MYFUNCS_H

