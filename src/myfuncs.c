#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// Define a function pointer type for comparison functions
typedef int (*compare_func)(void*, void*);
// Comparison functions
int compare_int_min(void* a, void* b) {
    return (*(int*)a < *(int*)b);
}
int compare_int_max(void* a, void* b) {
    return (*(int*)a > *(int*)b);
}
int compare_double_min(void* a, void* b) {
    return (*(double*)a < *(double*)b);
}
int compare_double_max(void* a, void* b) {
    return (*(double*)a > *(double*)b);
}
void* find_extreme(void* array, size_t element_size, size_t num_elements, compare_func comp) {
    void* extreme = array;

    for (size_t i = 1; i < num_elements; ++i) {
        void* current_element = (char*)array + i * element_size;
        if (comp(current_element, extreme)) {
            extreme = current_element;
        }
    }

    return extreme;
}
// check the start ID of elements in the mesh file 
int checkEIDS(int *elems){
    size_t int_size = sizeof(elems) / sizeof(elems[0]);
    // Find min and max for int array
    int* int_min = (int*)find_extreme(elems, sizeof(int), int_size, compare_int_min);
    //printf("--> ID of elements start from %d!\n",*int_min);
    return *int_min;
}
// assign an integer array to a pointer
int assignIntArray(int **ptr, int *arr, int size) {
    *ptr = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        (*ptr)[i] = arr[i];
    }
    return 0;
}
// assign a double array to a pointer
int assignDoubleArray(double **ptr, double *arr, int size) {
    *ptr = (double *)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        (*ptr)[i] = arr[i];
    }
    return 0;
}
// check the start # for the element 
// make data structure for elements surrounded a point
int save_esurp(int npoin,int nelem,int *elems,int **esurp2,int **esurp_pointer2,int Nredge){
    int e=0;
    // check the start ID element 
        if (checkEIDS(elems)!=1){
            fprintf(stderr,"ERROR: The element ID should start from 1 for save_esurp function!\n");
            return -1;
        }
	// define parameter
	int *pointer,*pointer2,*esurp;
	// allocate memory for pointer
		pointer=calloc(npoin+2,sizeof(*(pointer)));
		pointer2=calloc(npoin+2,sizeof(*(pointer2)));
	// find the nr of Elements surround each point
		for(int ele=0;ele<nelem;ele++) {
			for(int i=0;i<Nredge;i++) pointer[elems[Nredge*ele+i]+1]++;
		}

		for(int pt=1;pt<=npoin;pt++) pointer[pt+1]+=pointer[pt];
		for(int pt=0;pt<=npoin+1;pt++) pointer2[pt]=pointer[pt];
	// allocate memory for the esurp
		esurp=malloc(pointer[npoin+1]*sizeof(*(esurp)));	

	// find elements surround each point
		for(int ele=0;ele<nelem;ele++) {
			for (int i=0;i<Nredge;i++) esurp[pointer[elems[Nredge*ele+i]]++]=ele;
		} 
	
	// done
	free(pointer);
	*esurp2=esurp;
	*esurp_pointer2=pointer2;
    return e;
}
// find neighbor element arround each element from esurp data structure
int *find_nei_elem3D(int *esurp_pointer,int *esurp,int *num_nei, int *open,int *elems, int ele, int ele_p1, int ele_p2,int Nredge){

	int elemnum,*p,*order;
	int *nei;//if nei[0] is -9999 it means that there is problem to find a neighbour element ----------->nei[1] indicate to the number of neighbour in the nei[0]	
	nei=calloc(2, sizeof(*(nei)));
	nei[0]=-9999;
	p=calloc(Nredge,sizeof(*p));
	order=calloc(2*Nredge,sizeof(*order));
	for (int i=1;i<Nredge;i++) {
		order[2*i-1]=i;
		order[2*i]=i;
	}	
/* find neighbour of ele */
	int *lesps; //list of element around ele_p1 and ele_p2
	int j=0;
	int nr=esurp_pointer[ele_p1+1]+esurp_pointer[ele_p2+1]-esurp_pointer[ele_p1]-esurp_pointer[ele_p2];
	lesps=calloc(nr,sizeof(*lesps));
	for (int i = esurp_pointer[ele_p1]; i < esurp_pointer[ele_p1+1]; i++) {
		lesps[j]=esurp[i];
		j++;}
	for (int i = esurp_pointer[ele_p2]; i < esurp_pointer[ele_p2+1]; i++) {
		lesps[j]=esurp[i];
		j++;}

	for (int i = 0; i < nr; i++) {

		elemnum = lesps[i];
		if (num_nei[elemnum]<Nredge && open[elemnum]==0){

			if (elemnum==ele) continue; // checking the same element ID
			
			for (int j=0;j<Nredge;j++) p[j]=elems[Nredge*elemnum+j];
			for (int j=0;j<Nredge;j++){
				if (p[order[2*j]]==ele_p1&&p[order[2*j+1]]==ele_p2){
					nei[0]=elemnum;
					nei[1]=j;
					break;
				}
				if(p[order[2*j]]==ele_p2&&p[order[2*j+1]]==ele_p1){
					nei[0]=elemnum;
					nei[1]=j;
					break;
				}
			}
        }	
	}
	free(lesps);
	return nei;
}
// make data structure for elements surrounded an element 
int save_esure(int nelem,int *elems,int *esurp_pointer,int *esurp,int **esue2, int **open2,int Nredge){
	int e=0;
    // check the start ID element 
        if (checkEIDS(elems)!=1){
            fprintf(stderr,"ERROR: The element ID should start from 1 for save_esurp function!\n");
            return -1;
        }
    //parameters
	int *p,*order,*nei,*num_nei,*out,*open;

	/* Allocate space to nei pointer */
		p=calloc(Nredge,sizeof(*p));
		order=calloc(2*Nredge,sizeof(*order));
		nei = calloc(Nredge*nelem, sizeof(*(nei)));
		num_nei=calloc(nelem, sizeof(*(num_nei)));
		open = calloc(nelem, sizeof(*(open)));
	// initializing 
		for (int ele = 0; ele < nelem; ele++){
			for (int j=0;j<Nredge;j++) nei[Nredge*ele+j]=-1;
		}
		for (int i=1;i<Nredge;i++) {
			order[2*i-1]=i;
			order[2*i]=i;
		}

	for (int ele = 0; ele < nelem; ele++){
		for (int j=0;j<Nredge;j++) p[j]=elems[Nredge*ele+j];

		// controller condition
		if (num_nei[ele]==Nredge) continue; 
		for (int j=0;j<Nredge;j++){
			if (nei[Nredge*ele+j]==-1){	
			out=find_nei_elem3D(esurp_pointer,esurp,num_nei,open,elems,ele,p[order[2*j]],p[order[2*j+1]],Nredge);
			if (out[0]!=-9999){
				nei[Nredge*ele+j]=out[0];
				num_nei[ele]++;
				num_nei[out[0]]++;
				nei[Nredge*out[0]+out[1]]=ele;
			}else{
				nei[Nredge*ele+j]=-2;
			}
			}
		}
		//find the element adjacent to hole
		if (num_nei[ele]<Nredge){
			open[ele]=1;
			//printf("the element %d is near holes\n",ele);
		}
			
	}
	// Done;
	free (num_nei);
	*esue2=nei;
	*open2=open;
    return e;	
}
