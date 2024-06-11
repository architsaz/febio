#ifndef MYFUNCS_H
#define MYFUNCS_H
// Define a function pointer type for comparison functions
typedef int (*compare_func)(void*, void*);
int assignIntArray(int **ptr, int *arr, int size);
int  assignDoubleArray(double **ptr, double *arr, int size);
int save_esurp(int npoin,int nelem,int *elems,int **esurp2,int **esurp_pointer2,int Nredge);
void* find_extreme(void* array, size_t element_size, size_t num_elements, compare_func comp);
int checkEIDS(int *elems);
#endif // MYFUNCS_H

