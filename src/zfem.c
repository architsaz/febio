#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "mystructs.h"
#include "common.h"
#include "myfuncs.h"
#include "febiofuncs.h"

int readgz_zfem(char *path,int *npoin, int *nelem, double **ptxyz,int **elems) {
int e=0;
// defined arrayes and varables
    int npoin1,nelem1,*elems1;
    double *ptxyz1;
    ptxyz1 = NULL;
    elems1 = NULL;

/* Allocate space to File pointer */
    gzFile fptr = gzopen(path, "r");

    if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
        return -1;
    }
    else {
        printf("  File opened - %s.\n", path);
    }
    printf("%s\n",path);
    
/* Read all lines of the file */
    int buffer = 100;
    char *str;
    char line[buffer];

    int endcount = 0;
    int nscan;
    int iline;

    char  test[20];
        // initializing memory:
            npoin1=0;
            nelem1=0;	

    while(1){
    // start reading points:	
        str = edit_endlinegz_character(line, buffer, fptr);
        nscan = sscanf(str, "%s",test);
        if(!strcmp(test,"POINTS")){

            printf("    Reading POINTS.\n");
                    if (nscan != 1) {
                        fprintf(stderr,"ERROR: Incorrect number of entries on POINTS line.\n");
                        return -1;
                    }
                /* Read Number of Points */	    
                    str = edit_endlinegz_character(line, buffer, fptr);     
                    nscan = sscanf(str, "%d",&(npoin1));
                    printf("      Number of Points = %d.\n", npoin1);
                        if (nscan != 1) {
                            fprintf(stderr,"ERROR: Incorrect number of entries on Number of Points line.\n");
                            return -1;
                        }

                /* Read Coordinates of all points */
                        ptxyz1 = malloc((size_t)dimension*(size_t)npoin1 * sizeof(*(ptxyz1)));				      
                    for (iline = 0; iline < npoin1; iline++) {
                        str = edit_endlinegz_character(line, buffer, fptr);
                            nscan = sscanf(str, "%lf %lf %lf",
                                        &(ptxyz1[dimension*iline + 0]),&(ptxyz1[dimension*iline + 1]),&(ptxyz1[dimension*iline + 2]));
                                    if (nscan != 3) {
                                        fprintf(stderr,"ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline+1);
                                        return(e++);
                                    }
                                    // printf("nscan = %d, iline = %d. %lf, %lf, %lf.\n",
                                    // 	       	nscan, iline,
                                    // 	       	ptxyz1[dimension*iline+0], ptxyz1[dimension*iline+1], ptxyz1[dimension*iline+2]);  
                        }     
                        printf("    Done Reading cordination of points (iline = %d)(dimension= %d).\n\n\n", iline,dimension);
                        endcount += 1;
        }
    // start reading elements:

        if (!strcmp(test,"TRIANGLE")){
            printf("    Reading ELEMENTS.\n");
            str = edit_endlinegz_character(line, buffer, fptr);
            nscan = sscanf(str, "%s",test);
            if (!strcmp(test,"mesh")){
                printf("    Starting to read conectivity of elements.\n");
            }
            /* Read Number of Elements */	
                str = edit_endlinegz_character(line, buffer, fptr);
                nscan = sscanf(str, "%d",&(nelem1));
                printf("      Number of ELEMENTS = %d.\n", nelem1);
                
                if (nscan != 1) {
                    fprintf(stderr,"ERROR: Incorrect number of entries on Number of ELEMENTS number.\n");
                    return -1;
                }

            /* Read Connectivity of all elements */
                elems1 = malloc(3 * (size_t)nelem1 * sizeof(*(elems1)));
                
                for (iline = 0; iline < nelem1; iline++) {
                
                str = edit_endlinegz_character(line, buffer, fptr);
                        nscan = sscanf(str, "%d %d %d",
                                    &(elems1[3*iline]), &(elems1[3*iline + 1]),&(elems1[3*iline + 2]));
                            
                            if (nscan != 3) {
                                fprintf(stderr,"ERROR: Incorrect number of conectinity of elements on line %d th of elements.\n", iline+1);
                                return -1;
                            }
                                // printf("nscan = %d, iline = %d,\t %d,\t %d,\t %d.\n",
                            //  						nscan, iline,
                            //  						elems1[3*iline+0], elems1[3*iline+1], elems1[3*iline+2]);	
                }	
                                            printf("    Done Reading connectinity of elements (iline = %d).\n\n\n", iline);
                printf("      Number of Triangular elements = %d.\n", nelem1);
                endcount += 1;		      		
        }
            
        if (endcount == 2) {
        printf("  Done Reading Nodes.\n\n\n");
            break;
        }    
    }
	if (gzclose(fptr) == EOF) {
        // If fclose returns EOF, it means there was an error closing the file
    	printf("Error closing %s\n",path);
        return -1;
    }
    *npoin=npoin1;
    *nelem=nelem1;
    *elems=elems1;
    *ptxyz=ptxyz1;  
/* return */
    printf("*  Exiting function for reading %s file.\n\n",path);
    checkEIDS(elems1);
    return e;
}
int read_zfem(char *path,int *npoin, int *nelem, double **ptxyz,int **elems) {
int e=0;
// defined arrayes and varables
    static int npoin1,nelem1,*elems1;
    static double *ptxyz1;
    ptxyz1 = NULL;
    elems1 = NULL;

/* Allocate space to File pointer */
    FILE *fptr;
    fptr = calloc(1, sizeof(*fptr));

/* Opening File */
    fptr = fopen(path, "r");

    if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
        return -1;
    }
    else {
        printf("  File opened - %s.\n", path);
    }
    printf("%s\n",path);
    
/* Read all lines of the file */
    int buffer = 100;
    char *str;
    char line[buffer];

    int endcount = 0;
    int nscan;
    int iline;

    char  test[20];
        // initializing memory:
            npoin1=0;
            nelem1=0;	

    while(1){
    // start reading points:	
        str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%s",test);
        if(!strcmp(test,"POINTS")){

            printf("    Reading POINTS.\n");
                    if (nscan != 1) {
                        fprintf(stderr,"ERROR: Incorrect number of entries on POINTS line.\n");
                        return -1;
                    }
                /* Read Number of Points */	    
                    str = edit_endline_character(line, buffer, fptr);     
                    nscan = sscanf(str, "%d",&(npoin1));
                    printf("      Number of Points = %d.\n", npoin1);
                        if (nscan != 1) {
                            fprintf(stderr,"ERROR: Incorrect number of entries on Number of Points line.\n");
                            return -1;
                        }

                /* Read Coordinates of all points */
                        ptxyz1 = malloc((size_t)dimension*(size_t)npoin1 * sizeof(*(ptxyz1)));				      
                    for (iline = 0; iline < npoin1; iline++) {
                        str = edit_endline_character(line, buffer, fptr);
                            nscan = sscanf(str, "%lf %lf %lf",
                                        &(ptxyz1[dimension*iline + 0]),&(ptxyz1[dimension*iline + 1]),&(ptxyz1[dimension*iline + 2]));
                                    if (nscan != 3) {
                                        fprintf(stderr,"ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline+1);
                                        return(e++);
                                    }
                                    // printf("nscan = %d, iline = %d. %lf, %lf, %lf.\n",
                                    // 	       	nscan, iline,
                                    // 	       	ptxyz1[dimension*iline+0], ptxyz1[dimension*iline+1], ptxyz1[dimension*iline+2]);  
                        }     
                        printf("    Done Reading cordination of points (iline = %d)(dimension= %d).\n\n\n", iline,dimension);
                        endcount += 1;
        }
    // start reading elements:

        if (!strcmp(test,"TRIANGLE")){
            printf("    Reading ELEMENTS.\n");
            str = edit_endline_character(line, buffer, fptr);
            nscan = sscanf(str, "%s",test);
            if (!strcmp(test,"mesh")){
                printf("    Starting to read conectivity of elements.\n");
            }
            /* Read Number of Elements */	
                str = edit_endline_character(line, buffer, fptr);
                nscan = sscanf(str, "%d",&(nelem1));
                printf("      Number of ELEMENTS = %d.\n", nelem1);
                
                if (nscan != 1) {
                    fprintf(stderr,"ERROR: Incorrect number of entries on Number of ELEMENTS number.\n");
                    return -1;
                }

            /* Read Connectivity of all elements */
                elems1 = malloc(3 * (size_t)nelem1 * sizeof(*(elems1)));
                
                for (iline = 0; iline < nelem1; iline++) {
                
                str = edit_endline_character(line, buffer, fptr);
                        nscan = sscanf(str, "%d %d %d",
                                    &(elems1[3*iline]), &(elems1[3*iline + 1]),&(elems1[3*iline + 2]));
                            
                            if (nscan != 3) {
                                fprintf(stderr,"ERROR: Incorrect number of conectinity of elements on line %d th of elements.\n", iline+1);
                                return -1;
                            }
                                // printf("nscan = %d, iline = %d,\t %d,\t %d,\t %d.\n",
                            //  						nscan, iline,
                            //  						elems1[3*iline+0], elems1[3*iline+1], elems1[3*iline+2]);	
                }	
                                            printf("    Done Reading connectinity of elements (iline = %d).\n\n\n", iline);
                printf("      Number of Triangular elements = %d.\n", nelem1);
                endcount += 1;		      		
        }
            
        if (endcount == 2) {
        printf("  Done Reading Nodes.\n\n\n");
            break;
        }    
    }
	if (fclose(fptr) == EOF) {
        // If fclose returns EOF, it means there was an error closing the file
    	printf("Error closing %s\n",path);
        return -1;
    }
    *npoin=npoin1;
    *nelem=nelem1;
    *elems=elems1;
    *ptxyz=ptxyz1;  
/* return */
    printf("*  Exiting function for reading flds.zfem file.\n\n");
    checkEIDS(elems1);
    return e;
}
int read_BCmask(char *path,mesh *M,int **BCmask2){
    static int *arr;
    arr=calloc((size_t)M->nelem,sizeof(*arr));
    FILE *fptr;
    fptr=calloc(1,sizeof(*fptr));
    fptr = fopen(path, "r");
	if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
        return -1;
  	}
  	else {
   	    printf("  File opened - %s.\n", path);
  	}
    /* Read all lines of the file */
	int buffer = 100;
	char *str,line[buffer];
	int nscan,iline;
	for (iline = 0; iline<M->nelem;iline++){
	    str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%d",&arr[iline]);
        if (nscan!=1) {
            fprintf(stderr,"ERROR: there is error in line %d of file %s.\n",iline,path);
            exit(EXIT_FAILURE);
        }
    }
    *BCmask2=arr;
    return 0;
}
int read_wallmask(char *path,mesh *M,input *inp,int **Melem2) {
    int e=0;
	int *Melem;
	Melem = calloc((size_t)M->nelem, sizeof(*(Melem)));
/* Allocate space to File pointer */
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));
/* Opening File */
	fptr = fopen(path, "r");
	if (fptr == NULL) {
   	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
   	return -1;
  	}
  	else {
   	printf("  File opened - %s.\n", path);
  	} 
/* Read all lines of the file */
	int buffer = 100;
	char *str,line[buffer];
	int nscan,iline;
/* Read labes of all elements */
	int temp=0;
	for (iline = 0; iline<M->nelem;iline++){
	str = edit_endline_character(line, buffer, fptr);
    nscan = sscanf(str, "%d",&temp);
    if (nscan != 1) {
        fprintf(stderr,"ERROR: Incorrect number of entries on POINTS line.\n");
        return -1;
    }
    
    //printf("label : %d %d %d \n",inp->label[0],inp->label[1],inp->label[2]);

    // check the value of the Melem: (remove rupture and cyan color from dataset)
    	for (int k=0;k<inp->label_num;k++) {if (temp==inp->label[k]) Melem[iline]=inp->label[k];};
	    //if (iline<10)printf("\t\t the lable of element %d is:\t%d should be : %d\n.",iline,Melem[iline],temp);  
	}
	//printf("All of lables was readed.");
	if (fclose(fptr) == EOF) {
        // If fclose returns EOF, it means there was an error closing the file
    	printf("Error closing %s\n",path);
        return -1;
    }  
/* done */
*Melem2=Melem;
printf("*  Exiting function for reading .WALL mask file!\n\n");
return e;
}
int read_regionmask(char *path,mesh *M,input *inp,int **region_id2, int **region_idp2) {
    int e=0;
    // the id of each portion @ labels_surf.zfem : 
    // dome : 	16
    // body :	8
    // neck : 	4
    // parent arteries : 1 
    // distal arteries : 2	

    int *region_id;
    region_id = malloc((size_t)M->npoin * sizeof(*(region_id)));

    // Allocate space to File pointer 
    FILE *fptr;
    fptr = calloc(1, sizeof(*fptr));

    /* Opening File */
    fptr = fopen(path, "r");
        if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
        return -1;
        }
        else{
        //printf("  File opened - %s.\n", path);
        }
        
    /* Read all lines of the file */
    int buffer = 100;
    char *str;
    char line[buffer];

    int endcount = 0;
    int nscan;
    int iline;

    char  test[20];
    // initializing memory:	
    while(1){
        str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%s",test);
        // start reading regions labels:	
        //if (!strcmp(test,"regions")){
        if (!strcmp(test,"regions")){	
            //printf("    Reading region mask file.\n");
            str = edit_endline_character(line, buffer, fptr);
            nscan = sscanf(str, "%s",test);
            //printf("%s\n",test);
            if (!strcmp(test,"1")){
                //printf("    Starting to read the label of each region.\n");
            }
                    
            for (iline = 0; iline < M->npoin; iline++) {   
            str = edit_endline_character(line, buffer, fptr);
            nscan = sscanf(str, "%d",&region_id[iline]);
            // check number			        
                if (nscan != 1) {
                    fprintf(stderr,"ERROR: Incorrect number of conectinity of elements on line %d th of elements.\n", iline+1);
                    return -1;
                }
            // check value of mask with colorid array: 
            int checkflag=0;
            for (int k=0; k<(inp->colorid_num);k++) {if(region_id[iline]==inp->colorid[k]){checkflag++; break;}}		
            if (checkflag!=1) {printf("ERROR: the value of element %d (value=%d) is not in the colorId array\n",iline,region_id[iline]);return -1;};	

            }

            //printf("    Done Reading labels of regional mask (iline = %d).\n\n\n", iline);
            endcount += 1;		      		
        }
                
        if (endcount == 1) {
            printf("* Done Reading region mask file!\n");
            break;
            }    
        }

        /* free(line); */
        if (fclose(fptr) == EOF) {
            // If fclose returns EOF, it means there was an error closing the file
            printf("Error closing %s\n",path);
            return -1;
        }
        int *region_id_ele,ele;
    
        region_id_ele=malloc((size_t)M->nelem*sizeof(*(region_id_ele)));

        int points[3]={0,0,0};
        for (ele=0;ele<M->nelem;ele++){
        points[0]=M->elems[3*ele];
        points[1]=M->elems[3*ele+1];
        points[2]=M->elems[3*ele+2];
        //region_id_ele[ele]=(region_id[points[0]-1]+region_id[points[1]-1]+region_id[points[2]-1])/3;
        // to avoid the bug in average id color for element :
        region_id_ele[ele]=region_id[points[0]-1];
        }
        
        /* return */
        *region_id2=region_id_ele;
        *region_idp2=region_id;
        printf("*  Exiting function for reading regional mask file.\n\n");
    return e;    
}
