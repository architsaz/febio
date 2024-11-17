#ifndef MYFUNCS_H
#define MYFUNCS_H

#include <zlib.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
// Define a function pointer type for comparison functions
int check_winding_order(int nelem, int *elems, double *ptxyz);
char *edit_endline_character(char *, int, FILE *);
char *edit_endlinegz_character(char *line, int buffer, gzFile file);
int checkEIDS(int *);
int ConverMesh(mesh *, mesh *, ConvertorFunc);
void tri3_to_tri6(mesh *, mesh **);
void tri3_to_quad4(mesh *, mesh **);
void SCA_int_VTK(FILE *, char *, int, int, void *);
void SCA_double_VTK(FILE *, char *, int, int, void *);
void VEC_double_VTK(FILE *, char *, int, int, void *);
void read_VTK_double(FILE *fptr, int col, int nr, void **field);
void read_VTK_int(FILE *fptr, int col, int nr, void **field);
int SaveVTK(char *, char *, int, mesh *, elemVTK, FunctionWithArgs *, size_t, FunctionWithArgs *, size_t);
int ReadVTK(char *, char *, int, FunctionWithArgs2 *, int);
void tri3funcVTK(FILE *, int, int *);
void tri6funcVTK(FILE *, int, int *);
int countline(char *);
void inserthash(HashTable *table, const char *key, const char *value);
char *gethash(HashTable *table, const char *key);
void freeTable(HashTable *table);
int compare_int_min(void *a, void *b);
int compare_int_max(void *a, void *b);
int compare_double_min(void *a, void *b);
int compare_double_max(void *a, void *b);
void *find_extreme(void *array, size_t element_size, size_t num_elements, compare_func comp);
int assignIntArray(int **ptr, int *arr, int size);
int assignDoubleArray(double **ptr, double *arr, int size);
int *find_nei_elem3D(int *esurp_pointer, int *esurp, int *num_nei, int *open, int *elems, int ele, int ele_p1, int ele_p2, int Nredge);
int save_esurp(int npoin, int nelem, int *elems, int **esurp2, int **esurp_pointer2, int Nredge);
int save_esure(int nelem, int *elems, int *esurp_pointer, int *esurp, int **esue2, int **open2, int Nredge);
int save_fsure(int nelem, int *esure, int **efid2, int *numf, int Nredge);
int save_psurf(int nelem, int numf, int *elems, int *esure, int **psurf2, int Nredge);
int save_esurf(int nelem, int *esure, int numf, int **esurf2, int Nredge);
int save_normele(int nelem, int *elems, double *ptxyz, double **norm);
unsigned int hash(const char *key);
Entry *createEntry(const char *key, const char *value);
char *my_strdup(const char *s);
int jacobiMethod(int nelem, double *tensor, double **eigenvalue1, double **eigenvector1);
void saveMultipleArraysToFile(const char *path, int numArrays, void *arrays[], int sizes[], DataType types[], const char *headers[]);
int findMaxSize(int numArrays, int sizes[]);
// Function to calculate statistuic features
double calculate_mean(double *arr, int size, double *weight);
double calculate_median(double *, int);
double find_max(double *, int);
double find_min(double *, int);
double calculate_stddev(double arr[], int size, double mean, double *weight);
void sort_array(double *, int);
int calc_area_tri3(double *ptxyz, int *elems, int nelem, double **area2);
double sumarr(double *arr, int size);
int save_centri3(int nelem, int *elems, double *ptxyz, double **cen2);
int normalize(double *v);
int transpose(double *src, int row, int col, double **dest2);
int mat_mult(double *A, int Arow, int Acol, double *B, int Brow, int Bcol, double **C2);
// int inverse_classic_GE(double *A, int n, double **inverse1);
double determinant(double *matrix, int n);
void barycentric(double *p, double *a, double *b, double *c, double *u, double *v, double *w);
void interpolate_vector(double *p, double *a, double *b, double *c, double *va, double *vb, double *vc, double *result);
int find_zero_magnitude(double *a, double *b, double *c, double *va, double *vb, double *vc, double *zero_point);
double trace(double A[3][3]);
double traceA2(double A[3][3]);
double determinant3by3(double A[3][3]);
int solveCubic(double a, double b, double c, Complex roots[3]);
int calc_comp_eigen(double A[3][3], Complex roots[3]);
double vector_magnitude(double *v);
// int pseudo_inverse_GE(double *A, int m, int n, double **pseudo_inv);
// int gaussian_elimination(double *A, int m, int n, double *I);
int inverse_classic_GEPP(double *matrix, double *inverse, int n);
int inverse_matrix_SVD(double *mat, double *mat_inv, int n);
int find_critic_vec(mesh *M1, double *vec, int num_zero, double **zero_ptxyz2, int **type_zero_ele2, int **type_zero_p);

#endif // MYFUNCS_H
