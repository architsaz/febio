#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include "mystructs.h"
#include "common.h"
#include "myfuncs.h"

// Comparison functions
char *edit_endline_character(char *line, int buffer, FILE *fptr) {

	char *str;
	int len;

	str = fgets(line, buffer, fptr);
	len = (int)strlen(str);
	if(str[len-1] == '\n') str[len-1] = '\0';

	return str;
}
char *edit_endlinegz_character(char *line, int buffer, gzFile fptr) {

	char *str;
	int len;

	str = gzgets(fptr,line, buffer);
	len = (int)strlen(str);
	if(str[len-1] == '\n') str[len-1] = '\0';

	return str;
}
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
    size_t int_size = sizeof(* elems) / sizeof(elems[0]);
    // Find min and max for int array
    int* int_min = (int*)find_extreme(elems, sizeof(int), int_size, compare_int_min);
    //printf("--> ID of elements start from %d!\n",*int_min);
    return *int_min;
}
// assign an integer array to a pointer
int assignIntArray(int **ptr, int *arr, int size) {
    *ptr = (int *)malloc((size_t)size * sizeof(int));
    for (int i = 0; i < size; i++) {
        (*ptr)[i] = arr[i];
    }
    return 0;
}
// assign a double array to a pointer
int assignDoubleArray(double **ptr, double *arr, int size) {
    *ptr = (double *)malloc((size_t)size * sizeof(double));
    for (int i = 0; i < size; i++) {
        (*ptr)[i] = arr[i];
    }
    return 0;
}
// check the start # for the element 
// make data structure for elements surrounding a point
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
		pointer=calloc((size_t)npoin+2,sizeof(*(pointer)));
		pointer2=calloc((size_t)npoin+2,sizeof(*(pointer2)));
	// find the nr of Elements surround each point
		for(int ele=0;ele<nelem;ele++) {
			for(int i=0;i<Nredge;i++) pointer[elems[Nredge*ele+i]+1]++;
		}

		for(int pt=1;pt<=npoin;pt++) pointer[pt+1]+=pointer[pt];
		for(int pt=0;pt<=npoin+1;pt++) pointer2[pt]=pointer[pt];
	// allocate memory for the esurp
		esurp=malloc((size_t)pointer[npoin+1]*sizeof(*(esurp)));	

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
	static int *nei;//if nei[0] is -9999 it means that there is problem to find a neighbour element ----------->nei[1] indicate to the number of neighbour in the nei[0]	
	nei=calloc((size_t)2, sizeof(*(nei)));
	nei[0]=-9999;
	p=calloc((size_t)Nredge,sizeof(*p));
	order=calloc(2*(size_t)Nredge,sizeof(*order));
	for (int i=1;i<Nredge;i++) {
		order[2*i-1]=i;
		order[2*i]=i;
	}	
/* find neighbour of ele */
	int *lesps; //list of element around ele_p1 and ele_p2
	int j=0;
	int nr=esurp_pointer[ele_p1+1]+esurp_pointer[ele_p2+1]-esurp_pointer[ele_p1]-esurp_pointer[ele_p2];
	lesps=calloc((size_t)nr,sizeof(*lesps));
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
			
			for (int k=0;k<Nredge;k++) p[k]=elems[Nredge*elemnum+k];
			for (int k=0;k<Nredge;k++){
				if (p[order[2*k]]==ele_p1&&p[order[2*k+1]]==ele_p2){
					nei[0]=elemnum;
					nei[1]=k;
					break;
				}
				if(p[order[2*k]]==ele_p2&&p[order[2*k+1]]==ele_p1){
					nei[0]=elemnum;
					nei[1]=k;
					break;
				}
			}
        }	
	}
	free(lesps);
	free(order);
	free(p);
	return nei;
}
// make data structure for elements surrounding an element 
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
		p=calloc((size_t)Nredge,sizeof(*p));
		order=calloc(2*(size_t)Nredge,sizeof(*order));
		nei = calloc((size_t)Nredge*(size_t)nelem, sizeof(*(nei)));
		num_nei=calloc((size_t)nelem, sizeof(*(num_nei)));
		open = calloc((size_t)nelem, sizeof(*(open)));
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
	free (num_nei);free(p);free(order);
	*esue2=nei;
	*open2=open;
    return e;	
}
// find Nr of eadge in the mesh and make data structure for adges surrounding an element
int save_fsure(int nelem, int *esure, int **efid2,int *numf,int Nredge){
    int e=0;
    static int *efid;
	int nei,ele,f;
    int num=0;

    // allocate memory
    efid= calloc((size_t)Nredge*(size_t)nelem, sizeof(*efid));
    for (int i=0;i<(Nredge*nelem);i++) efid[i]= -1;
    
    for (ele=0; ele<nelem; ele++){
        for (f=0; f<Nredge; f++){
            if (efid[Nredge*ele+f] < 0){
                nei= esure[Nredge*ele+f];
                if (nei>=0){  // this is not boundary face 
                efid [Nredge*ele+f] = num;
                for (int j=0;j<Nredge;j++){	
                	if(esure[Nredge*nei+j]==ele)  efid[Nredge*nei+j]=num;
                }
                }else{ // this is on the boundary face
                efid [Nredge*ele+f] = nei;
                }
                num++;
            }
        }
    }
    printf("* nr face : %d\n", num);
    *numf=num;
    *efid2=efid;
    return e;
}
// make data structure for points surrounding an edge 
int save_psurf(int nelem, int numf, int *elems,int *esure, int **psurf2,int Nredge){
    int e=0;
    int *psurf,*order;
    int *efid,nei,*p;
    int num = 0;

    // allocate memory
    p=calloc((size_t)Nredge,sizeof(*p));
    order=calloc(2*(size_t)Nredge,sizeof(*order));
    efid= calloc((size_t)Nredge*(size_t)nelem, sizeof(*efid));
    for (int i=0;i<(Nredge*nelem);i++) efid[i]= -1;
    psurf= calloc(2*(size_t)numf, sizeof(*psurf));
	
	for (int i=1;i<Nredge;i++) {
			order[2*i-1]=i;
			order[2*i]=i;
	}

    for (int ele=0; ele<nelem; ele++){
        for (int f=0; f<Nredge; f++){
            if (efid[Nredge*ele+f] < 0){
                for (int j=0;j<Nredge;j++) p[j]=elems[Nredge*ele+j];
                for (int j=0;j<Nredge;j++){
	                if (f==j) {psurf[2*num]=p[order[2*j]];psurf[2*num+1]=p[order[2*j+1]];}
	            }
                nei= esure[Nredge*ele+f];
                if (nei>=0){  // this is not boundary face
                efid [Nredge*ele+f] = num;
                for (int j=0;j<Nredge;j++){	
                	if(esure[Nredge*nei+j]==ele)  efid[Nredge*nei+j]=num;
                }
                }
                num++;
            }
        }
    }

    *psurf2=psurf;
	free(efid);free(p);free(order);
    printf("* psurf is done.\n");
    return e;
}
// make data structure for elements surrounding an edge
int save_esurf(int nelem,int *esure, int numf, int **esurf2,int Nredge){

    int e=0;
    int *esurf;
    int *efid,nei,ele,f;
    int num = 0;

    // allocate memory
    efid= calloc((size_t)Nredge*(size_t)nelem, sizeof(*efid));
    for (int i=0;i<(Nredge*nelem);i++) efid[i]= -1;
    esurf= calloc(2*(size_t)numf, sizeof(*esurf));

    for (ele=0; ele<nelem; ele++){
        for (f=0; f<Nredge; f++){
            if (efid[Nredge*ele+f] < 0){
                nei= esure[Nredge*ele+f];
                esurf[2*num]=ele;
                esurf[2*num+1]=nei;
                if (nei>=0){  // this is not boundary face
                efid [Nredge*ele+f] = num;
                if(esure[Nredge*nei]==ele)  efid[Nredge*nei]=num;
                if(esure[Nredge*nei+1]==ele)  efid[Nredge*nei+1]=num;
                if(esure[Nredge*nei+2]==ele)  efid[Nredge*nei+2]=num;
                }
                num++;
            }
        }
    }
    *esurf2=esurf;
    printf("* esurf is done!\n");
    return e;
}
// conver mesh from tri3 to other type of mesh 
void tri3_to_tri6(mesh *M1,mesh **M2){

// define type of mesh for M2
    strcpy((*M2)->type,"tri");
    (*M2)->nredge=3;    
    (*M2)->nrpts=6;     
//  define coordinate and elems for M2        
	static double *ptxyz2;
	static int npoin2,*elems2,nelem2;
	    //allocate memmory
		nelem2=M1->nelem;
		npoin2=M1->numf+M1->npoin;
		ptxyz2=calloc(3*(size_t)npoin2,sizeof(*ptxyz2));
		elems2=calloc(6*(size_t)M1->nelem,sizeof(*elems2));

	    // coordinate of all(new+old) points
		for (int i=0;i<(3*M1->npoin);i++) ptxyz2[i]=M1->ptxyz[i];
		for (int i=0;i<M1->numf;i++){
			for (int j=0;j<3;j++){	
				ptxyz2[3*M1->npoin+3*i+j]=(M1->ptxyz[3*(M1->psurf[2*i]-1)+j]+M1->ptxyz[3*(M1->psurf[2*i+1]-1)+j])/2;
			}
		}
	    // 	new connectivity 
		for (int i=0;i<M1->nelem;i++){
			elems2[6*i+0]=M1->elems[3*i+0];		
			elems2[6*i+1]=M1->elems[3*i+1];
			elems2[6*i+2]=M1->elems[3*i+2];
			elems2[6*i+3]=M1->fsure[3*i+0]+M1->npoin+1;
			elems2[6*i+4]=M1->fsure[3*i+1]+M1->npoin+1;
			elems2[6*i+5]=M1->fsure[3*i+2]+M1->npoin+1;
		}
        //return:
        (*M2)->npoin=npoin2;
        (*M2)->elems=elems2;
        (*M2)->ptxyz=ptxyz2;
        (*M2)->nelem=nelem2;
// all other data structure same as M1
        (*M2)->Melem=M1->Melem; // wall charectristics from .wall file
        (*M2)->rpts=M1->rpts; // pointal value of regional mask     --> read labels_srf.zfem
        (*M2)->relems=M1->relems; // elemental value of regional mask --> approximate
printf("* the tri3 mesh converted to the tri6 mesh.\n- new npoin: %d\n- new nelem: %d\n",npoin2,nelem2);	
}
void tri3_to_quad4(mesh *M1,mesh **M2){
// define type of mesh for M2
    strcpy((*M2)->type,"quad");
    (*M2)->nredge=4;    
    (*M2)->nrpts=4;     
//  define coordinate and elems for M2  	
	double *ptxyz2;
	int npoin2,*elems2,nelem2,*Melem2,*relems2;

	//allocate memmory
		npoin2=M1->npoin+M1->nelem+M1->numf;
		nelem2=3*M1->nelem;
		ptxyz2=calloc(3*(size_t)npoin2,sizeof(*ptxyz2));
		elems2=calloc(4*(size_t)nelem2,sizeof(*elems2));
		Melem2=calloc((size_t)nelem2,sizeof(*Melem2));
		relems2=calloc((size_t)nelem2,sizeof(*relems2));

	// coordinate of all(new+old) points
		for (int i=0;i<(3*M1->npoin);i++) ptxyz2[i]=M1->ptxyz[i];
		for (int i=0;i<M1->nelem;i++){
			for (int j=0;j<3;j++){	
				ptxyz2[3*M1->npoin+3*i+j]=(M1->ptxyz[3*(M1->elems[3*i]-1)+j]+M1->ptxyz[3*(M1->elems[3*i+1]-1)+j]+M1->ptxyz[3*(M1->elems[3*i+2]-1)+j])/3;
			}
		}
		for (int i=0;i<M1->numf;i++){
			for (int j=0;j<3;j++){	
				ptxyz2[3*M1->npoin+3*M1->nelem+3*i+j]=(M1->ptxyz[3*(M1->psurf[2*i]-1)+j]+M1->ptxyz[3*(M1->psurf[2*i+1]-1)+j])/2;
			}
		}
	// 	new connectivity 
		for (int i=0;i<M1->nelem;i++){
			// first quadrilateral
			elems2[12*i+0*4+0]=M1->elems[3*i+0];	
			elems2[12*i+0*4+1]=M1->fsure[3*i+0]+M1->nelem+M1->npoin+1;
			elems2[12*i+0*4+2]=M1->npoin+i+1;
			elems2[12*i+0*4+3]=M1->fsure[3*i+2]+M1->nelem+M1->npoin+1;	
			Melem2[3*i]=M1->Melem[i];
			relems2[3*i]=M1->relems[i];

			// second quadrilateral
			elems2[12*i+1*4+0]=M1->elems[3*i+1];	
			elems2[12*i+1*4+1]=M1->fsure[3*i+1]+M1->nelem+M1->npoin+1;
			elems2[12*i+1*4+2]=M1->npoin+i+1;
			elems2[12*i+1*4+3]=M1->fsure[3*i+0]+M1->nelem+M1->npoin+1;			
			Melem2[3*i+1]=M1->Melem[i];
			relems2[3*i+1]=M1->relems[i];

			// third quadrilateral
			elems2[12*i+2*4+0]=M1->elems[3*i+2];	
			elems2[12*i+2*4+1]=M1->fsure[3*i+2]+M1->nelem+M1->npoin+1;
			elems2[12*i+2*4+2]=M1->npoin+i+1;
			elems2[12*i+2*4+3]=M1->fsure[3*i+1]+M1->nelem+M1->npoin+1;
			Melem2[3*i+2]=M1->Melem[i];
			relems2[3*i+2]=M1->relems[i];

		}
	//return:
        (*M2)->npoin=npoin2;
        (*M2)->elems=elems2;
        (*M2)->ptxyz=ptxyz2;
        (*M2)->nelem=nelem2;
// other data structure : 
	(*M2)->Melem=Melem2;
	(*M2)->relems=relems2;

printf("the tri3 mesh converted to the quad4 mesh.\n- new npoin: %d\n- new nelem: %d\n",npoin2,nelem2);	
}
int ConverMesh(mesh *M1,mesh *M2,ConvertorFunc Func){
    int e=0;
    // find element surround a point
    CHECK_ERROR(save_esurp(M1->npoin,M1->nelem,M1->elems,&M1->esurp,&M1->esurp_ptr,M1->nredge));
    //for (int i=0;i<20;i++) printf("%d \n",esurp_pointer[i]);  
// find element surround an element 
    CHECK_ERROR(save_esure(M1->nelem,M1->elems,M1->esurp_ptr,M1->esurp,&M1->esure,&M1->open,M1->nredge));
    //for (int i=0;i<M1->nelem;i++) printf("ele : %d e1: %d e2 : %d e3: %d\n",i,M1->esure[3*i],M1->esure[3*i+1],M1->esure[3*i+2]);
// find Nr of eadge and given id to adges*/
    CHECK_ERROR(save_fsure(M1->nelem,M1->esure,&M1->fsure,&M1->numf,M1->nredge)); 
    printf (" the number of face : %d \n",M1->numf);
    // for (int i=0; i<M1->nelem ; i++){
    //     printf("ele %d l1: %d l2: %d l3: %d \n",i,M1->fsure[3*i],M1->fsure[3*i+1],M1->fsure[3*i+2]);
    // } 
// find point surround a face*/
    CHECK_ERROR(save_psurf(M1->nelem,M1->numf,M1->elems,M1->esure,&M1->psurf,M1->nredge));
    // for (int i=0; i<M1->numf ; i++){
    //     printf("f %d p1: %d p2: %d \n",i,M1->psurf[2*i],M1->psurf[2*i+1]);
    // }
    Func(M1,&M2);
    return e;
}
void SCA_int_VTK(FILE *fptr,char *name,int col,int num,void *field){
	int* int_field = (int*)field;
	fprintf(fptr,"SCALARS %s int %d\nLOOKUP_TABLE default\n\n",name,col);
	for (int ie=0;ie<num;ie++){
		fprintf(fptr,"%d\n",int_field[ie]);
	}
}
void SCA_double_VTK(FILE *fptr,char *name,int col,int num,void *field){
	double* double_field = (double*)field;
	fprintf(fptr,"SCALARS %s double %d\nLOOKUP_TABLE default\n\n",name,col);
	for (int ie=0;ie<num;ie++){
		fprintf(fptr,"%lf\n",double_field[ie]);
	}
}
void tri3funcVTK(FILE *fptr,int nelem,int *elems){
	fprintf(fptr,"CELLS %d %d\n",nelem,4*nelem);
	for (int ie=0;ie<nelem;ie++){
		fprintf(fptr,"3 %d %d %d\n",elems[3*ie]-1,elems[3*ie+1]-1,elems[3*ie+2]-1);
	}
	fprintf(fptr,"\n");

	fprintf(fptr,"CELL_TYPES %d\n",nelem);
	for (int ie=0;ie<nelem;ie++){
		fprintf(fptr,"5\n");
	}
	fprintf(fptr,"\n");
}
void tri6funcVTK(FILE *fptr,int nelem,int *elems){
	fprintf(fptr,"CELLS %d %d\n",nelem,7*nelem);
	for (int ie=0;ie<nelem;ie++){
		fprintf(fptr,"6 %d %d %d %d %d %d\n",elems[6*ie]-1,elems[6*ie+1]-1,elems[6*ie+2]-1
		,elems[6*ie+3]-1,elems[6*ie+4]-1,elems[6*ie+5]-1);
	}
	fprintf(fptr,"\n");

	fprintf(fptr,"CELL_TYPES %d\n",nelem);
	for (int ie=0;ie<nelem;ie++){
		fprintf(fptr,"22\n");
	}
	fprintf(fptr,"\n");
}
void read_VTK_double(FILE *fptr,int col,int nr,void **field){
	void *arr;
	int buffer = 100;
    char line[buffer];
	char *token;
    const char delimiters[] = " \t\n"; // Delimiters: space, tab, and newline
    int nscan;
	arr = malloc((size_t)col*(size_t)nr * sizeof(double));
	for (int iline = 0; iline < nr; iline++) {
		if (fgets(line, buffer, fptr) == NULL) {
            if (feof(fptr)) {
                break;
            } else {
                exit(EXIT_FAILURE);
            }
        }
		nscan = 0;
		// Get the first token
			token = strtok(line, delimiters);

		// Continue getting tokens until NULL is returned
			while (token != NULL) {
				//printf("Token: %s\n", token);
				((double *)arr)[iline] = atof(token);
				//sscanf(token, "%lf", &arr[iline]);
				token = strtok(NULL, delimiters);
				nscan++;
			}
			if (nscan!=col) {
				fprintf(stderr,"ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline+1);
				exit(EXIT_FAILURE);
			}

	}
	*field=arr;
}
void read_VTK_int(FILE *fptr,int col,int nr,void **field){
	void *arr;
	int buffer = 100;
    char line[buffer];
	char *token;
    const char delimiters[] = " \t\n"; // Delimiters: space, tab, and newline
    int nscan;
	arr = malloc((size_t)col*(size_t)nr * sizeof(int));
	for (int iline = 0; iline < nr; iline++) {
		if (fgets(line, buffer, fptr) == NULL) {
            if (feof(fptr)) {
                break;
            } else {
                exit(EXIT_FAILURE);
            }
        }
		nscan = 0;
		// Get the first token
			token = strtok(line, delimiters);

		// Continue getting tokens until NULL is returned
			while (token != NULL) {
				//printf("Token: %s\n", token);
				//sscanf(token, "%d", &arr[iline]);
				((int *)arr)[iline]=atoi(token);
				token = strtok(NULL, delimiters);
				nscan++;
			}
			if (nscan!=col) {
				fprintf(stderr,"ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline+1);
				exit(EXIT_FAILURE);
			}

	}
	*field=arr;
}
int ReadVTK(char *dir, char *filenam,int step,FunctionWithArgs2 *prtfield,int countfield){
	int e = 0;
	char num[10];
    sprintf(num,"%d",step);
    char path[500];
    strcpy(path,dir);
	strcat(path,filenam);
    strcat(path,"_");
    strcat(path,num);
    strcat(path,".vtk");
	/* define File pointer:*/
        FILE *fptr;
        fptr = calloc(1, sizeof(*fptr));
		printf("open file - %s.\n", path);
    /* Opening File */
        fptr = fopen(path, "r");
        if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
        return -1;
        }
	/* Read all lines of the file */
    int buffer = 100;
    char *str;
    char line[buffer];
    int endcount = 0;

    char  test1[20], test[20];

	while(1){
		// start reading points:	
		str = edit_endline_character(line, buffer, fptr);
		sscanf(str, "%s %s ",test1,test);
		for (int ifield=0;ifield<countfield;ifield++){
			if(!strcmp(test,prtfield[ifield].name)){
				printf("    Reading %s.\n",prtfield[ifield].name);
				/* Read header of field */	    
					str = edit_endline_character(line, buffer, fptr);
					sscanf(str, "%s",test1);
					if(!strcmp(test1,"LOOKUP_TABLE")) {
						str = edit_endline_character(line, buffer, fptr);
						sscanf(str, "%s",test1);
					}
					if(!strcmp(test1,"")) {
						str = edit_endline_character(line, buffer, fptr);
					}

				/* Read value of field */
				prtfield[ifield].function(fptr,prtfield[ifield].col,prtfield[ifield].nr,prtfield[ifield].arr);     
				endcount += 1;
			} 
		}
		if (endcount == countfield) {
			printf("  Done Reading all %d fields.\n",countfield);
			break;
		}
	}
	if (fclose(fptr) == EOF) {
        // If fclose returns EOF, it means there was an error closing the file
    	printf("Error closing %s\n",path);
        return -1;
    }
    		
	return e;
}
int SaveVTK(char *dir, char *filenam,int step,mesh *M,elemVTK elemfunc,FunctionWithArgs elefuncs[], size_t nrelefield,FunctionWithArgs pntfuncs[], size_t nrpntfield){
	int e=0;
	// check the start ID element 
	if (checkEIDS(M->elems)!=1){
		fprintf(stderr,"ERROR: The element ID should start from 1 for SaveVTK function!\n");
		return -1;
	}	
	char num[10];
    sprintf(num,"%d",step);
    char path[500];
    strcpy(path,dir);
	strcat(path,filenam);
    strcat(path,"_");
    strcat(path,num);
    strcat(path,".vtk");
	char command[500];
	strcpy(command,"rm ");
	strcat(command,path);
	/* define File pointer:*/
        FILE *fptr;
        fptr = calloc(1, sizeof(*fptr));
    /* Opening File */
        fptr = fopen(path, "w");
        if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
        return -1;
        }
    /*write the header of file : */ 
        fprintf(fptr,"# vtk DataFile Version 3.0\n");
        fprintf(fptr,"3D Unstructured Surface Grid  with %s%d mesh type\n",M->type,M->nrpts);
        fprintf(fptr,"ASCII\n\n");
	/*write the position of file : */ 
        fprintf(fptr,"DATASET UNSTRUCTURED_GRID\n");
        fprintf(fptr,"POINTS %d float\n",M->npoin);
        for (int ip=0;ip<M->npoin;ip++){
            fprintf(fptr,"%lf %lf %lf\n",M->ptxyz[3*ip],M->ptxyz[3*ip+1],M->ptxyz[3*ip+2]);
        }
        fprintf(fptr,"\n");
	/*write the elems and cell type : */
		elemfunc(fptr,M->nelem,M->elems);
       
	// write SCALER pointal fields in the file: 
		if (nrpntfield!=0)fprintf(fptr,"POINT_DATA %d\n",M->npoin);
		for (size_t i = 0; i < nrpntfield; ++i) {
        	pntfuncs[i].function(fptr, pntfuncs[i].name,pntfuncs[i].col,pntfuncs[i].nr,pntfuncs[i].field); // Call each function with its array and size
    	}
	// write SCALER elemental fields in the file: 
		if (nrelefield!=0)fprintf(fptr,"CELL_DATA %d\n",M->nelem);
		for (size_t i = 0; i < nrelefield; ++i) {
        	elefuncs[i].function(fptr, elefuncs[i].name,elefuncs[i].col,elefuncs[i].nr,elefuncs[i].field); // Call each function with its array and size
    	//printf("field %ld done.\n",i);
		}		
	if (fclose(fptr) == EOF) {
        // If fclose returns EOF, it means there was an error closing the file
    	printf("Error closing %s\n",path);
        return -1;
    }	
	printf ("* wrote %s in the VTK format!\n",path);	
	return e;
}
int countline(char *path) {
  FILE *fp;
  int count = 0;  // Initialize line counter
  int c;  // To store a character read from file

  // Open the file in read mode
  fp = fopen(path, "r");

  // Check if file exists
  if (fp == NULL) {
    printf("Error: Could not open file\n");
    return 1;
  }

  // Read contents of file
  while ((c = fgetc(fp)) != EOF) {
    if ((char)c == '\n') {  // Increment count if newline character is encountered
      count++;
    }
  }

  // Print the number of lines
  //printf("The number of lines in the file is: %d\n", count);

  return count;
}
// Hash function to convert a string key to an index
unsigned int hash(const char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash << 5) + (unsigned int)(*key++);
    }
    return hash % TABLE_SIZE;
}
// Custom strdup function if strdup is not available
char *my_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        memcpy(dup, s, len);
    }
    return dup;
}
// Function to create a new entry
Entry *createEntry(const char *key, const char *value) {
    Entry *entry = (Entry *)malloc(sizeof(Entry));
    entry->key = my_strdup(key);  // Duplicate the string
    entry->value = my_strdup(value);  // Duplicate the string
    entry->next = NULL;
    return entry;
}

// Function to insert a key-value pair into the hash table
void inserthash(HashTable *table, const char *key, const char *value) {
    unsigned int index = hash(key);
    Entry *entry = table->entries[index];
    if (entry == NULL) {
        // No collision, simply insert the new entry
        table->entries[index] = createEntry(key, value);
    } else {
        // Handle collision by chaining
        while (entry->next != NULL && strcmp(entry->key, key) != 0) {
            entry = entry->next;
        }
        if (strcmp(entry->key, key) == 0) {
            // Update the value if the key already exists
            free(entry->value);
            entry->value = my_strdup(value);
        } else {
            // Add a new entry at the end of the chain
            entry->next = createEntry(key, value);
        }
    }
}

// Function to retrieve a value by key
char *gethash(HashTable *table, const char *key) {
    unsigned int index = hash(key);
    Entry *entry = table->entries[index];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;  // Key not found
}

// Function to free the memory allocated for the hash table
void freeTable(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *entry = table->entries[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
}
