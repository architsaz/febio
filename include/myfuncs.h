#ifndef MYFUNCS_H
#define MYFUNCS_H

#include <zlib.h>
#include <string.h>
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
double sumarr(double *arr,int size);
#endif // MYFUNCS_H
