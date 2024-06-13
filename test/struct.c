#include <stdio.h>
#include <stdlib.h>

int n;
int *arr;

typedef struct {
    char type_mesh;
    char mesh_edge;
    int npoin;
    double *ptxyz;
    int nelem;
    int *elems;
    int *esurp;
    int *esure;
    int *esurf;
    int *psurf;
}mesh;
void input(int *npoin2,int **elems2){
    int npoin =4;
    int *elems;
    elems=calloc(4,sizeof(int*));
    elems[0]=2;
    *npoin2=4;
    *elems2=elems;
}
int main(){
    mesh *M1 = (mesh *)malloc(sizeof(mesh));
    if (M1 == NULL) {
        fprintf(stderr, "Memory allocation failed for mesh\n");
        return 1;
    }
    input(&M1->npoin,&M1->elems);
    n=M1->npoin;
    arr=M1->elems;
    printf("npoin: %d\n",M1->npoin);
    for (int i=0;i<M1->npoin;i++)
    printf("elems[%d]: %d\n",i,M1->elems[i]);
    printf("n: %d\n",n);
    for (int i=0;i<M1->npoin;i++)
    printf("arr[%d]: %d\n",i,arr[i]);
    return 0;
}