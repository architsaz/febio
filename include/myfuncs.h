#ifndef MYFUNCS_H
#define MYFUNCS_H
// Define a function pointer type for comparison functions
typedef int (*compare_func)(void*, void*);
int assignIntArray(int **ptr, int *arr, int size);
int  assignDoubleArray(double **ptr, double *arr, int size);
int save_esurp(int npoin,int nelem,int *elems,int **esurp2,int **esurp_pointer2,int Nredge);
void* find_extreme(void* array, size_t element_size, size_t num_elements, compare_func comp);
int checkEIDS(int *elems);
int save_esure(int nelem,int *elems,int *esurp_pointer,int *esurp,int **esue2, int **open2,int Nredge);
int save_fsure(int nelem, int *esure, int **efid2,int *numf,int Nredge);
int save_psurf(int nelem, int numf, int *elems,int *esure, int **psurf2,int Nredge);
int save_esurf(int nelem,int *esure, int numf, int **esurf2,int Nredge);
#endif // MYFUNCS_H

