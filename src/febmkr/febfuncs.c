// this code build input file according to the febio4
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>

// my library:
#include "mylib.h"

//information of input.txt file:
// Color ID for elements
int id_red;
int id_yel;
int id_whi;
int id_cya;
int id_rup;
int id_rem;
int id_bou;
int id_tra;
int id_ane;
// Young Modulus for each color ID
double y_red;
double y_yel;
double y_whi;
double y_rem;
double y_ane;
// Thickness for each color ID
double t_red;
double t_yel;
double t_whi;
double t_rem;
double t_ane;
//Density
double dens;
// Poisson Nr
double pois;
// pressure
double pre_pres;
double ulti_pres;

// information of center of inloet and outlets
int nr_cen;
double *center;	

void checkdir(char const *casename, char **path,char **path_surf2,char **path_label2,char **path_input2){
	// parameters and variables
	char path_achit[200] = "../../output";
	char slash[10]="/";
	char csd[10]="/csd/";
	strcat(path_achit,slash);
	strcat(path_achit,casename);
	strcat(path_achit,csd);	
	char fld_format[100]=".flds.zfem";
	char wall_format[100]=".wall";
	char input_format[100]="input.txt";
	char path_input[200];	
	char path_label[200];
	char path_surf[200];

	//build the path for label and surface file 
	strcpy(path_label,path_achit);
	strcat(path_label,casename);
	strcat(path_label,wall_format);
	printf("%s\n",path_label);
	strcpy(path_surf,path_achit);
	strcat(path_surf,casename);
	strcat(path_surf,fld_format);
	strcpy(path_input,path_achit);
	strcat(path_input,input_format);	

	//check availability wall file 
	if (file_exists(path_label)) printf("File %s exists\n", path_label);
    else {
    	printf("File %s doesn't exist.\n", path_label);
    	exit(EXIT_FAILURE);
    }

    if (file_exists(path_surf)) printf("File %s exists\n", path_surf);
    else {
    	printf("File %s doesn't exist.\n", path_surf);
    	exit(EXIT_FAILURE);
    }
    if (file_exists(path_input)) printf("File %s exists\n", path_input);
    else {
    	printf("File %s doesn't exist.\n", path_input);
    	exit(EXIT_FAILURE);
    }
    system("clear");
//done:
char path_temp[200];
strcpy(path_temp,path_achit);    
*path=path_temp;
*path_surf2=path_surf;
*path_label2=path_label;
*path_input2=path_input;
//printf("inside : %s\n",*path);
}
void read_regionmask(char const *casename, int nelem, int npoin, int *elems, int **region_id2, int **region_idp2) {

// the id of each portion @ labels_surf.zfem : 
// dome : 	16
// body :	8
// neck : 	4
// parent arteries : 2 
// distal arteries : 1	

	int *region_id;
	region_id = malloc(npoin * sizeof(*(region_id)));

// creat path:

	char path_achit[200] = "../../output/";
	char csd[10]="/csd/";
	strcat(path_achit,casename);
	strcat(path_achit,csd);	
	char name_regionmask[100]="labels_srf.zfem";
	char path_mask[200];

	strcpy(path_mask,path_achit);
	strcat(path_mask,name_regionmask);

// Allocate space to File pointer 
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));

/* Opening File */
	fptr = fopen(path_mask, "r");
	if (fptr == NULL) {
	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path_mask);
	exit(EXIT_FAILURE);
	}
	else{
	printf("  File opened - %s.\n", path_mask);
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
		    printf("    Reading region mask file.\n");
		  	str = edit_endline_character(line, buffer, fptr);
		  	nscan = sscanf(str, "%s",test);
		  	printf("%s\n",test);
		  	if (!strcmp(test,"1")){
		  		printf("    Starting to read the label of each region.\n");
		  	}
			      
			for (iline = 0; iline < npoin; iline++) {   
			str = edit_endline_character(line, buffer, fptr);
			nscan = sscanf(str, "%d",&region_id[iline]);			        
			if (nscan != 1) {
			fprintf(stderr,"ERROR: Incorrect number of conectinity of elements on line %d th of elements.\n", iline+1);
			exit(EXIT_FAILURE);
			}
			 // printf("nscan = %d, iline = %d,\t %d\n",
			 //  						nscan, iline,
			 //  						region_id[iline]);	
			 	    }	
			printf("    Done Reading labels of regional mask (iline = %d).\n\n\n", iline);
	      	endcount += 1;		      		
	    }
			  
		if (endcount == 1) {
		  	printf("  Done Reading region mask file.\n\n\n");
			break;
			}    
		}

		/* free(line); */
		fclose(fptr);
		int *region_id_ele;
		int ele,pt;
		int max;
		
		region_id_ele=malloc(nelem*sizeof(*(region_id_ele)));

		int points[3]={0,0,0};
		for (ele=0;ele<nelem;ele++){
		points[0]=elems[3*ele];
		points[1]=elems[3*ele+1];
		points[2]=elems[3*ele+2];
		//region_id_ele[ele]=(region_id[points[0]-1]+region_id[points[1]-1]+region_id[points[2]-1])/3;
		// to avoid the bug in average id color for element :
		region_id_ele[ele]=region_id[points[0]-1];
		}
	  
	  /* return */
	  *region_id2=region_id_ele;
	  *region_idp2=region_id;
	  printf("  Exiting function for reading regional mask file.\n\n");
}
void write_zfem_1intfield(char const *casename,int nelem,int npoin,int *elems,double *ptxyz, int *field){
char path[500];
//creat path :
	char path_achit[200] = "../../output/";
	char csd[10]="/csd/";
	strcat(path_achit,casename);
	strcat(path_achit,csd);	
	char zfem_format[100]="_onefield.zfem";	
	strcpy(path,path_achit);
	strcat(path,casename);
	strcat(path,zfem_format);	
	

// define paramteres
int ele,po,dim;
const char s[2] = ";";
	char *token;
	int num_feild=0;

/* initialization for write  concentration output data on file */
	FILE *fptr_value;
	fptr_value = calloc(1, sizeof(*fptr_value));
	// name of the file :		

	
	/* Opening File u*/
	fptr_value = fopen(path, "w");

	if(fptr_value == NULL) {
	    fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
	    exit(EXIT_FAILURE);
	}
	else {
	    printf("File opened - %s.\n", path);
	}
// writting the points coordination:	
	fprintf(fptr_value,"CONTINUOUS\nDATASET\nPOINTS\n%d\n",npoin);
		for(po=0;po<npoin;po++){
			fprintf(fptr_value,"\t%lf\t%lf\t%lf\n",ptxyz[dimension*po],ptxyz[dimension*po+1],ptxyz[dimension*po+2]);
		}
// writting a scaler field: 
	fprintf(fptr_value,"REAL\nlabel\n1\n");	
	for(po=0;po<npoin;po++){
	fprintf(fptr_value,"\t%d\n",field[po]);
	}		
		fprintf(fptr_value,"END_FIELDS\n");
// writting connectivity of elements:
 	
	fprintf(fptr_value,"TRIANGLE\nELEMENTS\n%d\n",nelem);
		for (ele=0;ele<nelem;ele++){
			fprintf(fptr_value,"\t%d\t%d\t%d\n",elems[3*ele],elems[3*ele+1],elems[3*ele+2]);
		}
	fprintf(fptr_value,"END\n");
fclose(fptr_value);

printf("* writting the %s in zfem format is finished.\n\n",path);			
}
void write_zfem_1doubfield(char const *casename,int nelem,int npoin,int *elems,double *ptxyz, double *field){
char path[500];
//creat path :
	char path_achit[200] = "../../output/";
	char csd[10]="/csd/";
	strcat(path_achit,casename);
	strcat(path_achit,csd);	
	char zfem_format[100]="_onefield.zfem";	
	strcpy(path,path_achit);
	strcat(path,casename);
	strcat(path,zfem_format);	

// define paramteres
int ele,po,dim;
const char s[2] = ";";
	char *token;
	int num_feild=0;

/* initialization for write  concentration output data on file */
	FILE *fptr_value;
	fptr_value = calloc(1, sizeof(*fptr_value));
	// name of the file :		

	/* Opening File u*/
	fptr_value = fopen(path, "w");

	if(fptr_value == NULL) {
	    fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
	    exit(EXIT_FAILURE);
	}
	else {
	    printf("File opened - %s.\n", path);
	}
// writting the points coordination:	
	fprintf(fptr_value,"CONTINUOUS\nDATASET\nPOINTS\n%d\n",npoin);
		for(po=0;po<npoin;po++){
			fprintf(fptr_value,"\t%lf\t%lf\t%lf\n",ptxyz[dimension*po],ptxyz[dimension*po+1],ptxyz[dimension*po+2]);
		}
// writting a scaler field: 
	fprintf(fptr_value,"REAL\nlabel\n1\n");	
	for(po=0;po<npoin;po++){
	fprintf(fptr_value,"\t%lf\n",field[po]);
	}		
		fprintf(fptr_value,"END_FIELDS\n");
// writting connectivity of elements:
 	
	fprintf(fptr_value,"TRIANGLE\nELEMENTS\n%d\n",nelem);
		for (ele=0;ele<nelem;ele++){
			fprintf(fptr_value,"\t%d\t%d\t%d\n",elems[3*ele],elems[3*ele+1],elems[3*ele+2]);
		}
	fprintf(fptr_value,"END\n");
fclose(fptr_value);

printf("* writting the %s in zfem format is finished.\n\n",path);			
}
void read_input(char const *casename,char *path) {
//creat path :
// char path_achit[200] = "../../output/";
// char csd[10]="/csd/";
// strcat(path_achit,casename);
// strcat(path_achit,csd);	
// char input_format[100]="input.txt";
char path_input[500];	

// strcpy(path_input,path_achit);
// strcat(path_input,input_format);

strcpy(path_input,path);
printf("%s\n",path_input);
printf("%s\n",path);
/* Allocate space to File pointer */
FILE *fptr;
fptr = calloc(1, sizeof(*fptr));


/* Opening File */
fptr = fopen(path_input, "r");

if (fptr == NULL) {
	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path_input);
	exit(EXIT_FAILURE);
	}
	else {
	printf("  File opened - %s.\n", path_input);
	}

/* Read all lines of the file */
int buffer = 500;
char *str;
char test[500];
char test2[20];
char line[buffer];

int endcount = 0;
int nscan, iline;

/* Reading all lines */
	while(1){
	
    str = edit_endline_character(line, buffer, fptr);
    if (!strcmp(str,"")) continue;
    sscanf(str, "%s",test);
    //printf("%s",test);
    // start reading casename to check input file:
	    if(!strcmp(test,"Casename")){
			/* checking the casename input file */	    
			    str = edit_endline_character(line, buffer, fptr);
			    nscan = sscanf(str, "%s",test2);     
				if(strcmp(test2,casename)){
				    fprintf(stderr,"ERROR: the casename indicate @ input file: %s is not match with casename in the argument.\n", casename);
					exit(EXIT_FAILURE);
				}
			endcount += 1;
		}
	// start reading labels IDs:
	    if(!strcmp(test,"Labels")){
	    	// save label IDs:
	    	str = edit_endline_character(line, buffer, fptr);
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%d %d %d %d %d %d %d %d %d",&(id_red),&(id_yel),&(id_whi),&(id_cya),&(id_rup),&(id_rem),&(id_bou),&(id_tra),&(id_ane));
			    if (nscan != 9) {
					fprintf(stderr,"ERROR: Incorrect number of labels @ input.txt file.\n");
					exit(EXIT_FAILURE);
			    }
			    printf("Labels:\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",id_red,id_yel,id_whi,id_cya,id_rup,id_rem,id_bou,id_tra,id_ane);
			endcount += 1;    
	    }
	//start reading Young modulus for each color
	    if(!strcmp(test,"Young")){
	    	// save Young Modulu:
	    	str = edit_endline_character(line, buffer, fptr);
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%lf %lf %lf %lf %lf",&(y_red),&(y_yel),&(y_whi),&(y_ane),&(y_rem));
			    if (nscan != 5) {
					fprintf(stderr,"ERROR: Incorrect number of Young modulus @ input.txt file.\n");
					exit(EXIT_FAILURE);
			    }
			    printf("Young Modulus:\n%lf\t%lf\t%lf\t%lf\t%lf\n",y_red,y_yel,y_whi,y_ane,y_rem);
			endcount += 1;  

	    }
	// start reading Thickness for each color	
	    if(!strcmp(test,"Thickness")){
	     	// save Thinkness:
	    	str = edit_endline_character(line, buffer, fptr);
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%lf %lf %lf %lf %lf",&(t_red),&(t_yel),&(t_whi),&(t_ane),&(t_rem));
			    if (nscan != 5) {
					fprintf(stderr,"ERROR: Incorrect number of Thickness @ input.txt file.\n");
					exit(EXIT_FAILURE);
			    }
			    printf("Thickness:\n%lf\t%lf\t%lf\t%lf\t%lf\n",t_red,t_yel,t_whi,t_ane,t_rem);
			endcount += 1; 

	    }
	// start reading density 
	    if(!strcmp(test,"Density")){

	    	// save Density:
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%lf",&(dens));
			    if (nscan != 1) {
					fprintf(stderr,"ERROR: Incorrect number of Density @ input.txt file.\n");
					exit(EXIT_FAILURE);
			    }
			    printf("Density:\n%lf\n",dens);
			endcount += 1; 

	    }	
	// start reading the poisson number
	    if(!strcmp(test,"Poisson")){
	    	// save Poisson nr:
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%lf",&(pois));
			    if (nscan != 1) {
					fprintf(stderr,"ERROR: Incorrect number of Poisson Nr @ input.txt file.\n");
					exit(EXIT_FAILURE);
			    }
			    printf("Poisson nr:\n%lf\n",pois);
			endcount += 1; 

	    }
	// start reading the poisson number
	    if(!strcmp(test,"Pressure")){
	    	// save Poisson nr:
	    	str = edit_endline_character(line, buffer, fptr);  
	    	str = edit_endline_character(line, buffer, fptr); 
	    	nscan = sscanf(str, "%lf %lf",&(pre_pres),&ulti_pres);
			    if (nscan != 2) {
					fprintf(stderr,"ERROR: Incorrect Pressure value @ input.txt file.\n");
					exit(EXIT_FAILURE);
			    }
			    printf("Pressure:\n%lf\t%lf\n",pre_pres,ulti_pres);
			endcount += 1; 

	    }    	
	// start reading the centers of inlets/outlets  
	    if(!strcmp(test,"Centers")){
				/* Read Number of inlets/outlets */	    
			      str = edit_endline_character(line, buffer, fptr);     
				  nscan = sscanf(str, "%d",&(nr_cen));
				  printf("      Number of inlet/outlet = %d.\n", nr_cen);
					    if (nscan != 1) {
					    	fprintf(stderr,"ERROR: Incorrect number of entries on Number of inlet/outlets line.\n");
							exit(EXIT_FAILURE);
					    }

				/* Read Coordinates of all center points */
				 	 	center = malloc(dimension*nr_cen*sizeof(*(center)));
				 	 	str = edit_endline_character(line, buffer, fptr);				      
				    for (iline = 0; iline < nr_cen; iline++) {
			        	str = edit_endline_character(line, buffer, fptr);
						    nscan = sscanf(str, "%lf %lf %lf",
									  &(center[dimension*iline + 0]),&(center[dimension*iline + 1]),&(center[dimension*iline + 2]));
									if (nscan != 3) {
									  fprintf(stderr,"ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline+1);
									  exit(EXIT_FAILURE);
									}
									printf("nscan = %d, iline = %d. %lf, %lf, %lf.\n",
										       	nscan, iline,
										       	center[dimension*iline+0], center[dimension*iline+1], center[dimension*iline+2]);  
						}     
				      	endcount += 1;
	    }	
	  
	if (endcount == 8) {
  	printf("  Done Reading input.txt.\n\n\n");
		break;
	}    
}

/* free(line); */
fclose(fptr);

/* return */
printf("  Exiting function for reading input file.\n\n");
}
void read_Mlable(char *path,int nelem, int *elems,int **Melem2) {
//creat path :
	// char path_achit[200] = "../../output/";
	// char csd[10]="/csd/";
	// strcat(path_achit,casename);
	// strcat(path_achit,csd);	
	// char wall_format[100]=".wall";	
	 char path_label[200];

	// strcpy(path_label,path_achit);
	// strcat(path_label,casename);
	// strcat(path_label,wall_format);

	 strcpy(path_label,path);
/* Allocate space to File pointer */
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));


/* Opening File */
	fptr = fopen(path_label, "r");

	if (fptr == NULL) {
   	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path_label);
   	exit(EXIT_FAILURE);
  	}
  	else {
   	printf("  File opened - %s.\n", path_label);
  	}
	  
/* Read all lines of the file */
	int buffer = 100;
	char *str;
	char line[buffer];
	int endcount = 0;
	int nscan;
	int iline;

/* Read labes of all elements */
	int *Melem;
	Melem = malloc(nelem * sizeof(*(elems)));
	for (iline = 0; iline<nelem;iline++){
	str = edit_endline_character(line, buffer, fptr);
    nscan = sscanf(str, "%d",&Melem[iline]);
    // check the value of the Melem: (remove rupture and cyan color from dataset)
    	if (Melem[iline]!=id_red && Melem[iline]!=id_yel && Melem[iline]!=id_whi) Melem[iline]=id_rem;
	//printf("\t\t the lable of element %d is:\t%d\n.",iline,Melem[iline]);  
	}

	//printf("All of lables was readed.");
/* free(line); */
fclose(fptr);
	  
/* done */
*Melem2=Melem;
printf("  Exiting function for reading label file.\n\n");
}
void read_aneulable(char const *casename,int nelem,int **anelem2, int *Melem) {
//creat path :
	char path_achit[200] = "../../output/";
	char csd[10]="/csd/";
	strcat(path_achit,casename);
	strcat(path_achit,csd);	
	char aneu_format[100]=".aneu.0.0";	
	char path_label[200];

	strcpy(path_label,path_achit);
	strcat(path_label,casename);
	strcat(path_label,aneu_format);

/* Allocate space to File pointer */
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));


/* Opening File */
	fptr = fopen(path_label, "r");

	if (fptr == NULL) {
   	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path_label);
   	exit(EXIT_FAILURE);
  	}
  	else {
   	printf("  File opened - %s.\n", path_label);
  	}
	  
/* Read all lines of the file */
	int buffer = 100;
	char *str;
	char line[buffer];
	int endcount = 0;
	int nscan;
	int iline;

/* Read labes of all elements */
	int *anelem;
	anelem = malloc(nelem * sizeof(*(anelem)));
	for (iline = 0; iline<nelem;iline++){
	str = edit_endline_character(line, buffer, fptr);
    nscan = sscanf(str, "%d",&anelem[iline]);
	//printf("\t\t the lable of element %d is:\t%d\n.",iline,anelem[iline]);  
	}
// update the Melem for the anurysm area which not colored:
	int ele;
	for (ele=0;ele<nelem;ele++){
		if (anelem[ele]==1 && Melem[ele]==0){
				Melem[ele]=id_ane;
				//printf("the ID element %d is change to %d\n",ele,id_ane);

		}
	}

	//printf("All of aneurysm lables was readed.\n");
/* free(line); */
fclose(fptr);
	  
/* done */
*anelem2=anelem;
printf("  Exiting function for reading label file.\n\n");
}
void material(int nelem, int *elems,int npoin,int *Melem, int *esurp,int *esurp_pointer,int nloop, double **E_ele2,double **E_fele2,double **E_nod2, double **t_ele2, double **t_fele2, double **t_nod2){

// parameter:
	int ele,pt,ifilter,nre,i;
	double sumy,sumt;
	int point[3]={0,0,0};
	double *E_ele;		// the Young Modulus for each element before appling Gaussian filter 
	double *E_fele; // the Young Modulus for each element after appling Gaussian filter
	double *E_nod; // the Young Modulus for each point after appling Gaussian filter	 	
	double *t_ele;		// the thickness for each element before appling Gaussian filter
	double *t_fele;		// the thickness for each element after appling Gaussian filter 
	double *t_nod;		// the thickness for each point after appling Gaussian filter
// allocate the memory for the E_ele and E_filter:
	E_ele=calloc(nelem,sizeof(*E_ele));
	E_fele=calloc(nelem,sizeof(*E_fele));
	E_nod=calloc(npoin,sizeof(*E_nod));
// allocate the memory for the thickness_ele and thickness_nod:
	t_ele=calloc(nelem,sizeof(*t_ele));
	t_fele=calloc(nelem,sizeof(*t_fele));
	t_nod=calloc(npoin,sizeof(*t_nod));	


//initialize the value of Young modulus & thickness for element and nodal:
	//element data:
		for (ele=0;ele<nelem;ele++){
			if (Melem[ele]==id_red){						
				E_ele[ele]=y_red;
				E_fele[ele]=y_red;	

				t_ele[ele]=t_red;
				t_fele[ele]=t_red;
				continue;										
			} 
			else if (Melem[ele]==id_yel){
				E_ele[ele]=y_yel;
				E_fele[ele]=y_yel;

				t_ele[ele]=t_yel;
				t_fele[ele]=t_yel;
				continue;												
			}
			else if (Melem[ele]==id_whi){
				E_ele[ele]=y_whi;
				E_fele[ele]=y_whi;

				t_ele[ele]=t_whi;
				t_fele[ele]=t_whi;
				continue;												
			}
			else if (Melem[ele]==id_ane){
				E_ele[ele]=y_ane;
				E_fele[ele]=y_ane;

				t_ele[ele]=t_ane;
				t_fele[ele]=t_ane;	
				continue;											
			} 					 
			else{
				E_ele[ele]=y_rem;
				E_fele[ele]=y_rem;

				t_ele[ele]=t_rem;
				t_fele[ele]=t_rem;							
			}
		}	
for (ifilter=0;ifilter<nloop;ifilter++){
// finding value of each nods by avarage value neighbour elements :		
	for(pt=1;pt<=npoin;pt++){
		sumy=0;sumt=0;
		nre=esurp_pointer[pt+1]-esurp_pointer[pt];
		for (i=0;i<nre;i++){
		sumy=sumy+E_fele[esurp[esurp_pointer[pt]+i]];
		sumt=sumt+t_fele[esurp[esurp_pointer[pt]+i]];
		}
		E_nod[pt-1]=sumy/nre;
		t_nod[pt-1]=sumt/nre;
	} 

// finding value of each element by average value nods:
	for (ele = 0; ele < nelem; ele++) {
		//if (Melem[ele]==id_tra){
		
		point[0]=elems[3*ele];
		point[1]=elems[3*ele+1];
		point[2]=elems[3*ele+2];
			
		E_fele[ele]=(E_nod[point[0]-1]+E_nod[point[1]-1]+E_nod[point[2]-1])/3;
		t_fele[ele]=(t_nod[point[0]-1]+t_nod[point[1]-1]+t_nod[point[2]-1])/3;
			//printf("%lf\n",mesh->thickness_filtered_ele[ele]);

		//}	
	}
} 

			printf("\t->material arrays built successfully \n");	

// done :
	*E_ele2=E_ele;
	*E_fele2=E_fele;
	*E_nod2=E_nod;
	*t_ele2=t_ele;
	*t_fele2=t_fele;
	*t_nod2=t_nod;
}
void boundary_labeling(int nelem, int *elems,int npoin, double *ptxyz, int *Melem,int *Mpoint,int *esure, int *region_id){
	//printf("inside the auto labeling function");
	int ele,edge,pt,point[3];

	/* Method 1 : find the boundary just by checking the center of open area in open_ended geometry */

	// int p1,p2,p3;
	// int neighbour_ele;
	// int num_outlet=0;
	// int *ele_boundary;
	// int initial_num_outlets=0;
	// int i,cent_ele;
	// double dist, dist_min;
	// double cx,cy,cz;
	


	// //allocate memory for ele_boundary:
	// 	ele_boundary=calloc(nelem,sizeof(ele_boundary));


	// // initializing the fist_ele_outlet with centers @ input file:
	// 	for (i=0;i<nr_cen;i++){
	// 		dist_min=100;
	// 		for(ele = 0; ele < nelem; ele++){
	// 			cx=(ptxyz[3*(elems[3*ele]-1)]+ptxyz[3*(elems[3*ele+1]-1)]+ptxyz[3*(elems[3*ele+2]-1)])/3;
	// 			cy=(ptxyz[3*(elems[3*ele]-1)+1]+ptxyz[3*(elems[3*ele+1]-1)+1]+ptxyz[3*(elems[3*ele+2]-1)+1])/3;
	// 			cz=(ptxyz[3*(elems[3*ele]-1)+2]+ptxyz[3*(elems[3*ele+1]-1)+2]+ptxyz[3*(elems[3*ele+2]-1)+2])/3;
	// 			//printf("initializing: %d\n",ele);
	// 			dist=sqrt(pow(cx-center[3*i],2)+pow(cy-center[3*i+1],2)+pow(cz-center[3*i+2],2));
	// 			if(dist<dist_min){
	// 				cent_ele=ele;
	// 				dist_min=dist;
	// 			}
	// 		}
	// 		// cx=(ptxyz[3*elems[3*cent_ele]]+ptxyz[3*elems[3*cent_ele+1]]+ptxyz[3*elems[3*cent_ele+2]])/3;
	// 		// cy=(ptxyz[3*elems[3*cent_ele]+1]+ptxyz[3*elems[3*cent_ele+1]+1]+ptxyz[3*elems[3*cent_ele+2]+1])/3;
	// 		// cz=(ptxyz[3*elems[3*cent_ele]+2]+ptxyz[3*elems[3*cent_ele+1]+2]+ptxyz[3*elems[3*cent_ele+2]+2])/3;
	// 		// printf("center\t%lf\t%lf\t%lf\n",center[3*i],center[3*i+1],center[3*i+2]);
	// 		// printf("%d\t%lf\t%lf\t%lf\n",cent_ele,cx,cy,cz);

	// 		Melem[cent_ele]=id_bou;
	// 		ele_boundary[i]=cent_ele;
	// 		//printf("element %d denoted as a boundary by chePen3D.\n",ele);
	// 	}


	// 	initial_num_outlets=nr_cen;
	// 	num_outlet=nr_cen;

	// // find other elements of boundaries:

	// 	for (ele = 0; ele < num_outlet; ele++){

	// 		//checking the normal vectors of neighbour elements:
	// 			for (edge=0;edge<3;edge++){
	// 				neighbour_ele=esure[3*ele_boundary[ele]+edge];
	// 				//printf("Neighbor of \t%dis:\t%d\n",ele_boundary[ele],neighbour_ele);
	// 				// first boundary elements determinded from the user
	// 				if(ele <= initial_num_outlets){
	// 					if(check_parallel(ele_boundary[ele],neighbour_ele)==1){
	// 						ele_boundary[num_outlet]=neighbour_ele;
	// 						num_outlet=num_outlet+1;
	// 						Melem[neighbour_ele]=id_bou;
	// 						//printf("element %d denoted as a boundary by check_parallel function.\n",neighbour_ele);
	// 						//printf("%d\n",ele);
	// 					}

	// 				}
	// 				// first boundary elements determinded from the 
	// 				if(ele > initial_num_outlets && Melem[neighbour_ele]!=id_bou){
	// 					if(check_parallel(ele_boundary[ele],neighbour_ele)==1){
	// 						ele_boundary[num_outlet]=neighbour_ele;
	// 						num_outlet=num_outlet+1;
	// 						Melem[neighbour_ele]=id_bou;
	// 						//printf("element %d denoted as a boundary by check_parallel function.\n",neighbour_ele);
	// 						//printf("%d\n",ele);
	// 					}
	// 				}
	// 			}

	// 	}

/* method 2 : take elements in the parent arteries as a boundary */

for (ele = 0; ele < nelem; ele++){
	if (region_id[ele]==2) Melem[ele]=id_bou;
}




// upgrade the nodal value for the boundary array:		
for (ele = 0; ele < nelem; ele++){

	point[0]=elems[3*ele];
	point[1]=elems[3*ele+1];
	point[2]=elems[3*ele+2];
	for (pt = 0; pt < 3; pt++) {
		if (Melem[ele]==id_bou)
		Mpoint[point[pt]-1]=id_bou;

	}
}
	
}
void write_feb3_prestain(char const *casename, char **runpath,int nelem, int *elems,int npoin, double *ptxyz,double *t_fele,double *E_fele,int *region_id, double *st,double pres_gradual, int iter){
	int nod, ele, bound,nodset_id,mat,nid,pid;
	char path[500];

	//creat path :
		char iterID[50]="_";
		char int2str [100];
		citoa(iter,int2str,10);
		strcat(iterID,int2str);
		char path_achit[200] = "../../output/";
		char csd[10]="/csd/";
		strcat(path_achit,casename);
		strcat(path_achit,csd);	
		char zfem_format[100]="_febio.feb";	

		strcpy(path,path_achit);
		strcat(path,casename);
		strcat(path,iterID);
		strcat(path,zfem_format);
	
	//write the feb file 
		FILE *fptr;
			fptr= calloc(1, sizeof(*fptr));

			/* Opening File */
			fptr= fopen(path, "w");

			if (fptr == NULL) {
			 	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
			 	exit(EXIT_FAILURE);
			}
			else {
			  printf("  File opened - %s.\n", path);
			}
			//writting a header porsion for *.feb file
				fprintf(fptr,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
				fprintf(fptr,"<febio_spec version=\"3.0\">\n");
			// writting a module porsion for *.feb file	
				fprintf(fptr,"\t<Module type=\"solid\"/>\n");
			// writting a control porsion for *.feb file
					fprintf(fptr,"\t<Control>\n");
						fprintf(fptr,"\t\t<analysis>STATIC</analysis>\n");
						 	fprintf(fptr,"\t\t<time_steps>10000</time_steps>\n");
						fprintf(fptr,"\t\t<step_size>0.0001</step_size>\n");
						fprintf(fptr,"\t\t<solver>\n");
							fprintf(fptr,"\t\t\t<max_refs>30</max_refs>\n");
							fprintf(fptr,"\t\t\t<max_ups>20</max_ups>\n");
							fprintf(fptr,"\t\t\t<diverge_reform>1</diverge_reform>\n");
							fprintf(fptr,"\t\t\t<reform_each_time_step>1</reform_each_time_step>\n");
							fprintf(fptr,"\t\t\t<dtol>0.00001</dtol>\n");
							fprintf(fptr,"\t\t\t<etol>0.0001</etol>\n");
							fprintf(fptr,"\t\t\t<rtol>0</rtol>\n");
							fprintf(fptr,"\t\t\t<lstol>0.9</lstol>\n");
							fprintf(fptr,"\t\t\t<min_residual>1e-15</min_residual>\n");
							fprintf(fptr,"\t\t\t<qnmethod>BFGS</qnmethod>\n");
							fprintf(fptr,"\t\t\t<rhoi>0</rhoi>\n");
							fprintf(fptr,"\t\t</solver>\n");
							fprintf(fptr,"\t\t<time_stepper>\n");
							fprintf(fptr,"\t\t\t<dtmin>0.001</dtmin>\n");
							fprintf(fptr,"\t\t\t<dtmax>0.1</dtmax>\n");
							fprintf(fptr,"\t\t\t<max_retries>10</max_retries>\n");
							fprintf(fptr,"\t\t\t<opt_iter>10</opt_iter>\n");
						fprintf(fptr,"\t\t</time_stepper>\n");
					fprintf(fptr,"\t</Control>\n");
					fprintf(fptr,"\t<Globals>\n");
						fprintf(fptr,"\t\t<Constants>\n");
							fprintf(fptr,"\t\t\t<T>0</T>\n\t\t\t<R>0</R>\n\t\t\t<Fc>0</Fc>\n");
						fprintf(fptr,"\t\t</Constants>\n");
					fprintf(fptr,"\t</Globals>\n");
			// writting a Material porsion for *.feb file		
					fprintf(fptr,"\t<Material>\n");


						fprintf(fptr,"\t\t<material id=\"1\" name=\"Material1\" type=\"prestrain elastic\">\n");
							fprintf(fptr,"\t\t\t<density>1101</density>\n");
							fprintf(fptr,"\t\t\t<elastic type=\"neo-Hookean\">\n");
							fprintf(fptr,"\t\t\t\t<E type=\"map\">map_E</E>\n");
							fprintf(fptr,"\t\t\t\t<v>%.3lf</v>\n",pois);
							fprintf(fptr,"\t\t\t</elastic>\n");
							fprintf(fptr,"\t\t\t<prestrain type=\"prestrain gradient\">\n");	
							fprintf(fptr,"\t\t\t\t<F0 type=\"map\">map_S</F0>\n");
							fprintf(fptr,"\t\t\t</prestrain>\n");
						fprintf(fptr,"\t\t</material>\n");		


					fprintf(fptr,"\t</Material>\n");
			// writting a Geometry porsion for *.feb file			
					fprintf(fptr,"\t<Mesh>\n");
					// writting a nodes porsion for *.feb file	
						fprintf(fptr,"\t\t<Nodes name=\"Object01\">\n");

							for (nod=0;nod<npoin;nod++){
								fprintf(fptr,"\t\t\t<node id=\"%d\">%lf,%lf,%lf</node>\n",nod+1,ptxyz[dimension*nod + 0]/100,
									ptxyz[dimension*nod + 1]/100,ptxyz[dimension*nod + 2]/100);
							}


						fprintf(fptr,"\t\t</Nodes>\n");
					// writting a elements porsion for *.feb file	
								
								fprintf(fptr,"\t\t<Elements type=\"tri3\" name=\"Part1\">\n");
								for (ele=0;ele<nelem;ele++){
										fprintf(fptr,"\t\t\t<elem id=\"%d\">%d,%d,%d</elem>\n",ele+1,elems[3*ele + 0],elems[3*ele + 1],
											elems[3*ele + 2]);
									}
								fprintf(fptr,"\t\t</Elements>\n");
							

					// writting boundary condition	
						fprintf(fptr,"\t\t<Surface name=\"FixedShellDisplacement1\">\n");	
						for (ele=0;ele<nelem;ele++){ 
									if (region_id[ele]<=1){
										fprintf(fptr,"\t\t\t<tri3 id=\"%d\">%d,%d,%d</tri3>\n",ele+1,elems[3*ele + 0],elems[3*ele + 1],
											elems[3*ele + 2]);
									}
						}	
					// writting pressure Load	
						fprintf(fptr,"\t\t</Surface>\n");
							fprintf(fptr,"\t\t<Surface name=\"PressureLoad1\">\n");
									for (ele=0;ele<nelem;ele++){ 
												if (region_id[ele]>1){
													fprintf(fptr,"\t\t\t<tri3 id=\"%d\">%d,%d,%d</tri3>\n",ele+1,elems[3*ele + 0],elems[3*ele + 1],
														elems[3*ele + 2]);
												}

									}	
							fprintf(fptr,"\t\t</Surface>\n");	

					fprintf(fptr,"\t</Mesh>\n");

					fprintf(fptr,"\t<MeshDomains>\n");
						fprintf(fptr,"\t\t<ShellDomain name=\"Part1\" mat=\"Material1\">\n");
							fprintf(fptr,"\t\t\t<shell_normal_nodal>1</shell_normal_nodal>\n");
						fprintf(fptr,"\t\t</ShellDomain>\n");	
					fprintf(fptr,"\t</MeshDomains>\n");

					fprintf(fptr,"\t<MeshData>\n");


						fprintf(fptr,"\t\t<ElementData var=\"shell thickness\" elem_set=\"Part1\">\n");
							for (ele=0;ele<nelem;ele++){
								fprintf(fptr,"\t\t\t<e lid=\"%d\">%lf,%lf,%lf</e>\n",ele+1,t_fele[ele],t_fele[ele],t_fele[ele]);		
							}
						fprintf(fptr,"\t\t</ElementData>\n");

						fprintf(fptr,"\t\t<ElementData name=\"map_E\" elem_set=\"Part1\">\n");
							for (ele=0;ele<nelem;ele++){
								fprintf(fptr,"\t\t\t<e lid=\"%d\">%lf</e>\n",ele+1,E_fele[ele]);		
							}
						fprintf(fptr,"\t\t</ElementData>\n");	

						fprintf(fptr,"\t\t<ElementData name=\"map_S\" datatype=\"mat3\" elem_set=\"Part1\">\n");
							for (ele=0;ele<nelem;ele++){
								fprintf(fptr,"\t\t\t<e lid=\"%d\">%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf</e>\n",ele+1,st[6*ele],st[6*ele+3],st[6*ele+4],st[6*ele+3],st[6*ele+1],st[6*ele+5],st[6*ele+4],st[6*ele+5],st[6*ele+2]);		
							}
						fprintf(fptr,"\t\t</ElementData>\n");						


					fprintf(fptr,"\t</MeshData>\n");
					// writting a Boundary porsion for *.feb file	
					fprintf(fptr,"\t<Boundary>\n");

						fprintf(fptr,"\t\t<bc name=\"FixedShellDisplacement1\" type=\"fix\" node_set=\"@surface:FixedShellDisplacement1\">\n");
							fprintf(fptr,"\t\t\t<dofs>sx,sy,sz</dofs>\n");
						fprintf(fptr,"\t\t</bc>\n");

					fprintf(fptr,"\t</Boundary>\n");
					// writting a Loads porsion for *.feb file
					fprintf(fptr,"\t<Loads>\n");
						
							fprintf(fptr,"\t\t<surface_load name=\"PressureLoad1\" type=\"pressure\" surface=\"PressureLoad1\">\n");

								fprintf(fptr,"\t\t\t<pressure lc=\"1\">%.5lf</pressure>\n",pres_gradual);
							
								fprintf(fptr,"\t\t\t<linear>0</linear>\n");
								fprintf(fptr,"\t\t\t<symmetric_stiffness>1</symmetric_stiffness>\n");
								fprintf(fptr,"\t\t\t<shell_bottom>0</shell_bottom>\n");
							fprintf(fptr,"\t\t</surface_load>\n");
						
					fprintf(fptr,"\t</Loads>\n");
					fprintf(fptr,"\t<LoadData>\n");

						fprintf(fptr,"\t\t<load_controller id=\"1\" type=\"loadcurve\">\n");
							fprintf(fptr,"\t\t\t<interpolate>SMOOTH</interpolate>\n");
							fprintf(fptr,"\t\t\t<points>\n");
								fprintf(fptr,"\t\t\t\t<point>0,0</point>\n");
								fprintf(fptr,"\t\t\t\t<point>1,1</point>\n");
							fprintf(fptr,"\t\t\t</points>\n");
						fprintf(fptr,"\t\t</load_controller>\n");	

					fprintf(fptr,"\t</LoadData>\n");

					fprintf(fptr,"\t<Output>\n");
						fprintf(fptr,"\t\t<plotfile type=\"febio\">\n");
							fprintf(fptr,"\t\t\t<var type=\"displacement\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"relative volume\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"shell strain\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"shell thickness\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"stress\"/>\n");
						fprintf(fptr,"\t\t</plotfile>\n");

						fprintf(fptr,"\t\t<logfile type=\"output\">\n");
							//fprintf(fptr,"\t\t\t<element_data data=\"sx;sy;sz;sxy;syz;sxz\">");

							//fprintf(fptr,"\t\t\t<element_data data=\"sx;sy;sz;sxy;syz;sxz\">");
							fprintf(fptr,"\t\t\t<element_data data=\"Ex;Ey;Ez;Exy;Exz;Eyz\">");

							fprintf(fptr,"</element_data>\n");
						fprintf(fptr,"\t\t</logfile>\n");
					fprintf(fptr,"\t</Output>\n");
				fprintf(fptr,"</febio_spec>\n");		



		printf("The feb format was written on path: \n%s\n",path);
		*runpath=path;
		fclose(fptr);
}
void write_feb4_prestain(char const *casename, char **runpath,int nelem, int *elems,int npoin, double *ptxyz,double *t_fele,double *E_fele,int *region_id, double *st,double pres_gradual, int iter){
	int nod, ele, bound,nodset_id,mat,nid,pid;
	char path[500];

	//creat path :
		char iterID[50]="_";
		char int2str [100];
		citoa(iter,int2str,10);
		strcat(iterID,int2str);
		char path_achit[200] = "../../output/";
		char csd[10]="/csd/";
		strcat(path_achit,casename);
		strcat(path_achit,csd);	
		char zfem_format[100]="_febio.feb";	

		strcpy(path,path_achit);
		strcat(path,casename);
		strcat(path,iterID);
		strcat(path,zfem_format);
	
	//write the feb file 
		FILE *fptr;
			fptr= calloc(1, sizeof(*fptr));

			/* Opening File */
			fptr= fopen(path, "w");

			if (fptr == NULL) {
			  fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
			  exit(EXIT_FAILURE);
			}
			else {
			  printf("  File opened - %s.\n", path);
			}
			//writting a header porsion for *.feb file
				fprintf(fptr,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
				fprintf(fptr,"<febio_spec version=\"4.0\">\n");
			// writting a module porsion for *.feb file	
				fprintf(fptr,"\t<Module type=\"solid\"/>\n");
			// writting a control porsion for *.feb file
					fprintf(fptr,"\t<Control>\n");
						fprintf(fptr,"\t\t<analysis>STATIC</analysis>\n");
						fprintf(fptr,"\t\t<time_steps>1</time_steps>\n");
						fprintf(fptr,"\t\t<step_size>1</step_size>\n");
						fprintf(fptr,"\t\t<plot_zero_state>0</plot_zero_state>\n");
						fprintf(fptr,"\t\t<plot_range>0,-1</plot_range>\n");
						fprintf(fptr,"\t\t<plot_level>PLOT_MAJOR_ITRS</plot_level>\n");
						fprintf(fptr,"\t\t<output_level>OUTPUT_MAJOR_ITRS</output_level>\n");
						fprintf(fptr,"\t\t<plot_stride>1</plot_stride>\n");
						fprintf(fptr,"\t\t<adaptor_re_solve>1</adaptor_re_solve>\n");
						fprintf(fptr,"\t\t<solver type=\"solid\">\n");
							fprintf(fptr,"\t\t\t<symmetric_stiffness>symmetric</symmetric_stiffness>\n");
							fprintf(fptr,"\t\t\t<equation_scheme>staggered</equation_scheme>\n");
							fprintf(fptr,"\t\t\t<equation_order>default</equation_order>\n");
							fprintf(fptr,"\t\t\t<optimize_bw>0</optimize_bw>\n");
							fprintf(fptr,"\t\t\t<lstol>0.9</lstol>\n");
							fprintf(fptr,"\t\t\t<lsmin>0.01</lsmin>\n");
							fprintf(fptr,"\t\t\t<lsiter>5</lsiter>\n");
							fprintf(fptr,"\t\t\t<max_refs>15</max_refs>\n");
							fprintf(fptr,"\t\t\t<check_zero_diagonal>0</check_zero_diagonal>\n");
							fprintf(fptr,"\t\t\t<zero_diagonal_tol>0</zero_diagonal_tol>\n");
							fprintf(fptr,"\t\t\t<force_partition>0</force_partition>\n");
							fprintf(fptr,"\t\t\t<reform_each_time_step>1</reform_each_time_step>\n");
							fprintf(fptr,"\t\t\t<reform_augment>0</reform_augment>\n");
							fprintf(fptr,"\t\t\t<diverge_reform>1</diverge_reform>\n");
							fprintf(fptr,"\t\t\t<min_residual>1e-20</min_residual>\n");
							fprintf(fptr,"\t\t\t<max_residual>0</max_residual>\n");
							fprintf(fptr,"\t\t\t<dtol>0.001</dtol>\n");
							fprintf(fptr,"\t\t\t<etol>0.01</etol>\n");
							fprintf(fptr,"\t\t\t<rtol>0</rtol>\n");
							fprintf(fptr,"\t\t\t<rhoi>-2</rhoi>\n");
							fprintf(fptr,"\t\t\t<alpha>1</alpha>\n");
							fprintf(fptr,"\t\t\t<beta>0.25</beta>\n");
							fprintf(fptr,"\t\t\t<gamma>0.5</gamma>\n");
							fprintf(fptr,"\t\t\t<logSolve>0</logSolve>\n");
							fprintf(fptr,"\t\t\t<arc_length>0</arc_length>\n");
							fprintf(fptr,"\t\t\t<arc_length_scale>0</arc_length_scale>\n");
							fprintf(fptr,"\t\t\t<qn_method type=\"BFGS\">\n");
									fprintf(fptr,"\t\t\t\t<max_ups>10</max_ups>\n");
									fprintf(fptr,"\t\t\t\t<max_buffer_size>0</max_buffer_size>\n");
									fprintf(fptr,"\t\t\t\t<cycle_buffer>1</cycle_buffer>\n");
									fprintf(fptr,"\t\t\t\t<cmax>100000</cmax>\n");
							fprintf(fptr,"\t\t\t</qn_method>\n");
						fprintf(fptr,"\t\t</solver>\n");
					fprintf(fptr,"\t</Control>\n");
					fprintf(fptr,"\t<Globals>\n");
						fprintf(fptr,"\t\t<Constants>\n");
							fprintf(fptr,"\t\t\t<T>0</T>\n\t\t\t<R>8.31446</R>\n\t\t\t<Fc>96485.3</Fc>\n");
						fprintf(fptr,"\t\t</Constants>\n");
					fprintf(fptr,"\t</Globals>\n");
			// writting a Material porsion for *.feb file		
					fprintf(fptr,"\t<Material>\n");


						fprintf(fptr,"\t\t<material id=\"1\" name=\"Material1\" type=\"prestrain elastic\">\n");
							fprintf(fptr,"\t\t\t<density>1101</density>\n");
							fprintf(fptr,"\t\t\t<elastic type=\"neo-Hookean\">\n");
							//fprintf(fptr,"\t\t\t\t<E type=\"map\">map_E</E>\n");
							fprintf(fptr,"\t\t\t\t<E>10000</E>\n");
							fprintf(fptr,"\t\t\t\t<v>%.3lf</v>\n",pois);
							fprintf(fptr,"\t\t\t</elastic>\n");
							fprintf(fptr,"\t\t\t<prestrain type=\"prestrain gradient\">\n");	
							fprintf(fptr,"\t\t\t\t<F0>1,0,0,0,1,0,0,0,1</F0>\n");
							//fprintf(fptr,"\t\t\t\t<F0 type=\"map\">map_S</F0>\n");
							fprintf(fptr,"\t\t\t</prestrain>\n");
						fprintf(fptr,"\t\t</material>\n");		


					fprintf(fptr,"\t</Material>\n");
			// writting a Geometry porsion for *.feb file			
					fprintf(fptr,"\t<Mesh>\n");
					// writting a nodes porsion for *.feb file	
						fprintf(fptr,"\t\t<Nodes name=\"Object01\">\n");

							for (nod=0;nod<npoin;nod++){
								fprintf(fptr,"\t\t\t<node id=\"%d\">%lf,%lf,%lf</node>\n",nod+1,ptxyz[dimension*nod + 0]/100,
									ptxyz[dimension*nod + 1]/100,ptxyz[dimension*nod + 2]/100);
							}


						fprintf(fptr,"\t\t</Nodes>\n");
					// writting a elements porsion for *.feb file	
								
								fprintf(fptr,"\t\t<Elements type=\"tri3\" name=\"Part1\">\n");
								for (ele=0;ele<nelem;ele++){
										fprintf(fptr,"\t\t\t<elem id=\"%d\">%d,%d,%d</elem>\n",ele+1,elems[3*ele + 0],elems[3*ele + 1],
											elems[3*ele + 2]);
									}
								fprintf(fptr,"\t\t</Elements>\n");
							

					// writting boundary condition
						pid=0;	
						fprintf(fptr,"\t\t<Surface name=\"FixedShellDisplacement1\">\n");	
							for (ele=0;ele<nelem;ele++){ 
								if (region_id[ele]==2){
									pid+=1;
									fprintf(fptr,"\t\t\t<tri3 id=\"%d\">%d,%d,%d</tri3>\n",pid,elems[3*ele + 0],elems[3*ele + 1],
										elems[3*ele + 2]);
								}
							}
						fprintf(fptr,"\t\t</Surface>\n");	
					// writting pressure Load	
						pid=0;
						fprintf(fptr,"\t\t<Surface name=\"PressureLoad1\">\n");
							for (ele=0;ele<nelem;ele++){ 
								if (region_id[ele]!=2){
									pid+=1;
									fprintf(fptr,"\t\t\t<tri3 id=\"%d\">%d,%d,%d</tri3>\n",pid,elems[3*ele + 0],elems[3*ele + 1],
										elems[3*ele + 2]);
								}
							}	
						fprintf(fptr,"\t\t</Surface>\n");	

					fprintf(fptr,"\t</Mesh>\n");

					fprintf(fptr,"\t<MeshDomains>\n");
						fprintf(fptr,"\t\t<ShellDomain name=\"Part1\" mat=\"Material1\">\n");
							// just for constant shell thickness ; 
							//fprintf(fptr,"\t\t\t<shell_thickness>0.0001</shell_thickness>\n");
						fprintf(fptr,"\t\t</ShellDomain>\n");	
					fprintf(fptr,"\t</MeshDomains>\n");


					fprintf(fptr,"\t<MeshData>\n");
						fprintf(fptr,"\t\t<ElementData type=\"shell thickness\" elem_set=\"Part1\">\n");
							for (ele=0;ele<nelem;ele++){
								fprintf(fptr,"\t\t\t<e lid=\"%d\">%lf,%lf,%lf</e>\n",ele+1,t_fele[ele],t_fele[ele],t_fele[ele]);		
							}
						fprintf(fptr,"\t\t</ElementData>\n");

					// 	fprintf(fptr,"\t\t<ElementData name=\"map_E\" elem_set=\"Part1\">\n");
					// 		for (ele=0;ele<nelem;ele++){
					// 			fprintf(fptr,"\t\t\t<e lid=\"%d\">%lf</e>\n",ele+1,E_fele[ele]);		
					// 		}
					// 	fprintf(fptr,"\t\t</ElementData>\n");	

					// 	fprintf(fptr,"\t\t<ElementData name=\"map_S\" elem_set=\"Part1\" datatype=\"mat3\">\n");
					// 		for (ele=0;ele<nelem;ele++){
					// 			fprintf(fptr,"\t\t\t<e lid=\"%d\">%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf</e>\n",ele+1,st[6*ele],st[6*ele+3],st[6*ele+4],st[6*ele+3],st[6*ele+1],st[6*ele+5],st[6*ele+4],st[6*ele+5],st[6*ele+2]);		
					// 		}
					// 	fprintf(fptr,"\t\t</ElementData>\n");						


					fprintf(fptr,"\t</MeshData>\n");

					// writting a Boundary porsion for *.feb file	
					fprintf(fptr,"\t<Boundary>\n");

						fprintf(fptr,"\t\t<bc name=\"FixedShellDisplacement1\" type=\"zero displacement\" node_set=\"@surface:FixedShellDisplacement1\">\n");
							fprintf(fptr,"<x_dof>1</x_dof>\n");
							fprintf(fptr,"<y_dof>1</y_dof>\n");
							fprintf(fptr,"<z_dof>1</z_dof>\n");
						fprintf(fptr,"\t\t</bc>\n");

					fprintf(fptr,"\t</Boundary>\n");
					// writting a Loads porsion for *.feb file
					fprintf(fptr,"\t<Loads>\n");
						
							fprintf(fptr,"\t\t<surface_load name=\"Pressure1\" type=\"pressure\" surface=\"PressureLoad1\">\n");

								fprintf(fptr,"\t\t\t<pressure lc=\"1\">%.5lf</pressure>\n",pres_gradual);
								
								fprintf(fptr,"\t\t\t<symmetric_stiffness>1</symmetric_stiffness>\n");
								fprintf(fptr,"\t\t\t<linear>0</linear>\n");
								fprintf(fptr,"\t\t\t<shell_bottom>1</shell_bottom>\n");
							fprintf(fptr,"\t\t</surface_load>\n");
						
					fprintf(fptr,"\t</Loads>\n");
					fprintf(fptr,"\t<LoadData>\n");

						fprintf(fptr,"\t\t<load_controller id=\"1\" name=\"LC1\" type=\"loadcurve\">\n");
							fprintf(fptr,"\t\t\t<interpolate>LINEAR</interpolate>\n");
							fprintf(fptr,"\t\t\t<extend>CONSTANT</extend>\n");
							fprintf(fptr,"\t\t\t<points>\n");
								fprintf(fptr,"\t\t\t\t<pt>0,0</pt>\n");
								fprintf(fptr,"\t\t\t\t<pt>1,1</pt>\n");
							fprintf(fptr,"\t\t\t</points>\n");
						fprintf(fptr,"\t\t</load_controller>\n");	

					fprintf(fptr,"\t</LoadData>\n");

					fprintf(fptr,"\t<Output>\n");
						fprintf(fptr,"\t\t<plotfile type=\"febio\">\n");
							fprintf(fptr,"\t\t\t<var type=\"displacement\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"relative volume\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"shell strain\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"shell thickness\"/>\n");
							fprintf(fptr,"\t\t\t<var type=\"stress\"/>\n");
						fprintf(fptr,"\t\t</plotfile>\n");

						// fprintf(fptr,"\t\t<logfile type=\"output\">\n");
						// 	//fprintf(fptr,"\t\t\t<element_data data=\"sx;sy;sz;sxy;syz;sxz\">");

						// 	//fprintf(fptr,"\t\t\t<element_data data=\"sx;sy;sz;sxy;syz;sxz\">");
						// 	fprintf(fptr,"\t\t\t<element_data data=\"Ex;Ey;Ez;Exy;Exz;Eyz\">");

						// 	fprintf(fptr,"</element_data>\n");
						// fprintf(fptr,"\t\t</logfile>\n");
					fprintf(fptr,"\t</Output>\n");
				fprintf(fptr,"</febio_spec>\n");		



		printf("The feb format was written on path: \n%s\n",path);
		*runpath=path;
		fclose(fptr);
}