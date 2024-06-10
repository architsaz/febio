#include <stdio.h>
#include <stdlib.h>

// Function to assign an integer array to a pointer
void assignIntArray(int **ptr, int *arr, int size) {
    *ptr = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        (*ptr)[i] = arr[i];
    }
}

// Function to assign a double array to a pointer
void assignDoubleArray(double **ptr, double *arr, int size) {
    *ptr = (double *)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        (*ptr)[i] = arr[i];
    }
}