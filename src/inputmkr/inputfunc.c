// include library:
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include <stdbool.h>

//information of FEBio_GeneralPara.txt file:
	// Color ID for elements
	int labl_red;
	int labl_yel;
	int labl_whi;
	int labl_cya;
	int labl_rup;
	int labl_rem;
	int labl_bou;
	int labl_tra;
	int labl_ane;
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


// include mylibrary:

#include "mylib.h"

void makedir(char const *casename){
	// finding the database and check the availablity 
		system("rm ../temp/path.txt 2>/dev/null");
		char command [500] = "find /dagon1/jcebral/aneuDB/db.R01/ -type d -maxdepth 2 -name \"";
		strcat(command,casename);
		char option [500] ="\" 2>/dev/null >../temp/path.txt";
		strcat (command,option);
		system(command); 
		printf("%s\n",command );

	    FILE *fp;
	        char *path = "../temp/path.txt";
	    long size;
	 
	    fp = fopen(path, "r");
	 
	    if (fp)
	    {
	        fseek (fp, 0, SEEK_END);
	        size = ftell(fp);
	        if (size ==0) {
	        	fprintf(stderr,"ERROR: Could not find directory in dagon for this case.\n");
	        	report(casename,0);
	        	exit(EXIT_FAILURE);
	        }
	        fclose(fp);
	    }
	    char path_newcase [100] = "../../output/";
		strcat(path_newcase,casename);

	//empty and build a directory for newcase
		char command4 [100]="rm -rf ";
		strcat(command4,path_newcase);
		char slash [100]="/*";
		//strcat(command4,slash);
		system(command4);

		char command3 [100]="mkdir ";
		strcat(command3,path_newcase);
		system(command3);

	//empty and build a directory for csd in the newcase directory
	    char command5 [100] = "rm -rf ";
	    char csd [100] ="/csd";
	    strcat(command5,path_newcase);
	    strcat(command5,csd);
	    strcat(command5,slash);
	    system(command5);
	    char command6 [100] = "rm -rf ";
	 	char post [100] ="/post";
	 	strcat(command6,path_newcase);
	 	strcat(command6,post);
	 	strcat(command6,slash);
	 	system(command6);

 	//empty and build a directory for post in the newcase directory
	    char command7 [100] = "mkdir ";
	    strcat(command7,path_newcase);
	    strcat(command7,csd);
	    system(command7);
	   	char command9 [100] = "mkdir ";
	   	char runfolder[10]="/run/";
	    strcat(command9,path_newcase);
	    strcat(command9,csd);
	    strcat(command9,runfolder);
	    system(command9);
	    char command8 [100] = "mkdir ";
	 	strcat(command8,path_newcase);
	 	strcat(command8,post);
	 	system(command8);
;

	 	system("clear");
 
}
void read_Generalinput(char const *casename) {
//creat path :
char path_input[200]= "../../../FEBio_GeneralPara.txt";	


/* Allocate space to File pointer */
FILE *fptr;
fptr = calloc(1, sizeof(*fptr));


/* Opening File */
fptr = fopen(path_input, "r");

if (fptr == NULL) {
	fprintf(stderr,"ERROR: Cannot open file - %s.\n", path_input);
	report(casename,0);
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
	// start reading labels IDs:
	    if(!strcmp(test,"Labels")){
	    	// save label IDs:
	    	str = edit_endline_character(line, buffer, fptr);
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%d %d %d %d %d %d %d %d %d",&(labl_red),&(labl_yel),&(labl_whi),&(labl_cya),&(labl_rup),&(labl_rem),&(labl_bou),&(labl_tra),&(labl_ane));
			    if (nscan != 9) {
					fprintf(stderr,"ERROR: Incorrect number of labels @ FEBio_GeneralPara.txt file.\n");
					report(casename,0);
					exit(EXIT_FAILURE);
			    }
			    printf("Labels:\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",labl_red,labl_yel,labl_whi,labl_cya,labl_rup,labl_rem,labl_bou,labl_tra,labl_ane);
			endcount += 1;    
	    }
	//start reading Young modulus for each color
	    if(!strcmp(test,"Young")){
	    	// save Young Modulu:
	    	str = edit_endline_character(line, buffer, fptr);
	    	str = edit_endline_character(line, buffer, fptr);  
	    	nscan = sscanf(str, "%lf %lf %lf %lf %lf",&(y_red),&(y_yel),&(y_whi),&(y_ane),&(y_rem));
			    if (nscan != 5) {
					fprintf(stderr,"ERROR: Incorrect number of Young modulus @ FEBio_GeneralPara.txt file.\n");
					report(casename,0);
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
					fprintf(stderr,"ERROR: Incorrect number of Thickness @ FEBio_GeneralPara.txt file.\n");
					report(casename,0);
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
					fprintf(stderr,"ERROR: Incorrect number of Density @ FEBio_GeneralPara.txt file.\n");
					report(casename,0);
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
					fprintf(stderr,"ERROR: Incorrect number of Poisson Nr @ FEBio_GeneralPara.txt file.\n");
					report(casename,0);
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
	    	nscan = sscanf(str, "%lf %lf",&(pre_pres),&(ulti_pres));
			    if (nscan != 2) {
					fprintf(stderr,"ERROR: Incorrect Pressure value @ FEBio_GeneralPara.txt file.\n");
					report(casename,0);
					exit(EXIT_FAILURE);
			    }
			    printf("Pressure:\n%lf\t%lf\n",pre_pres,ulti_pres);
			endcount += 1; 

	    }    		
	  
	if (endcount == 6) {
  	printf("  Done Reading FEBio_GeneralPara.txt.\n\n\n");
		break;
	}    
}

/* free(line); */
fclose(fptr);

/* return */
printf("  Exiting function for reading input file.\n\n");
}
void build_path (char const *casename,char **path_opensurf_achit, char **path_labels, char **path_input){
// parameters and variables
	char mdl [10] = "/mdl/";
	char slash[10] = "/";
	char space[10] = " ";
	char csd[10] = "/csd/";
	char etc[10] = "/etc/";
	char format_gz[5]=".gz";
	char pathdb[500];
	char name_opensurf[100];
	char path_opensurf[500];
	char path_regionmask[500];
	char path_regionmask_unzip[500];
	char name_zhemo[100]="zhemo.info.gz";
	char name_zhemo_unzip[100]="zhemo.info";
	char name_regionmask[100]="labels_srf.zfem.gz";
	char name_regionmask_unzip[100]="labels_srf.zfem";
	char path_zhemo[500];
	char path_opensurf2[500];
	char path_labels2[500];
	char path_zhemo_notgz[500];
	char path_achit[500] = "../../output/";
		strcat(path_achit,casename);
	char path_input2[500];
	char name_input[100]="input.txt";
	char path_zhemo_achit[500];
	char label[10]=".labels";
	char path_region[500]="/dagon1/jcebral/region/R01/wall";
	char fld_format[100]=".flds.zfem";
	char wall_format[100]=".wall";
	char aneu_format[100]=".aneu.0.0";

// build the main path to access to the jcebral directory
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));
	/* Opening File */
		fptr = fopen("../temp/path.txt", "r");

		if (fptr == NULL) {
			fprintf(stderr,"ERROR: Cannot open file - ../temp/path.txt.\n");
			report(casename,0);
			exit(EXIT_FAILURE);
		}	

	/* Read first lines inside the zhempo file */
		int buffer = 500;
		char *str;
		char line[buffer]; 
		str = edit_endline_character(line, buffer, fptr);
		sscanf(str, "%s",pathdb);	
		/* free(line); */
	 		fclose(fptr);
// region file:
	// build path for find zip file : 		
		strcpy(path_regionmask,pathdb);
		strcat(path_regionmask,etc);
		strcat(path_regionmask,name_regionmask);
	// copy & unzip region mask
		char command11	[500] = "cp ";
		if (file_exists(path_regionmask)){
			strcat(command11,path_regionmask);
			strcat(command11,space);
			strcat(command11,path_achit);
			strcat(command11,csd);
			system(command11);
			printf("\n");
			// unzip the REGION file at the CASENAME directoy
			char command12 [100] = "gzip -d ";
			strcat(command12,path_achit);
			strcat(command12,csd);
			strcat(command12,name_regionmask);
			system(command12);
		}	
		else{
			fprintf(stderr,"ERROR: there is not REGION MASK file in the %s/stc/\n",pathdb);
			report(casename,0);
			exit(EXIT_FAILURE);
		}


// Zhemo file:	 		
	// find zhemo file in the mdl directory:
	 	// if zhemo zip	
		 	strcpy(path_zhemo,pathdb);	
			strcat(path_zhemo,mdl);
			strcat(path_zhemo,name_zhemo);
		// if zhemo not zip
			strcpy(path_zhemo_notgz,pathdb);
			strcat(path_zhemo_notgz,mdl);
			strcat(path_zhemo_notgz,name_zhemo_unzip);
		//  path zhemo in the /dagon1/achitsaz/FEBio/
			strcpy(path_zhemo_achit,path_achit);
			strcat(path_zhemo_achit,csd);
			strcat(path_zhemo_achit,name_zhemo_unzip);

	// copy & unzip the zhemo info file into /dagon1/achitsaz/FEBio/casename :		
			char command1 [500] = "cp ";	
		//check is there gz format is availabe or not 
		if (file_exists(path_zhemo)){
			strcat(command1,path_zhemo);
			strcat(command1,space);
			strcat(command1,path_achit);
			strcat(command1,csd);
			system(command1);
			printf("\n");
			// unzip the zhemo file at the tmp local directoy
				char command2 [100] = "gzip -d ";
				strcat(command2,path_achit);
				strcat(command2,csd);
				strcat(command2,name_zhemo);
				//printf(command2);
				system(command2);
			}
		else if (file_exists(path_zhemo_notgz)){
			strcat(command1,path_zhemo_notgz);
			strcat(command1,space);
			strcat(command1,path_achit);
			strcat(command1,csd);
			system(command1);
			printf("\n");
			}
		else{
			fprintf(stderr,"ERROR: there is not zhemo file in the %s/mdl/\n",pathdb);
			report(casename,0);
			exit(EXIT_FAILURE);
		}
		// read the zhemo file 
			/* Allocate space to File pointer */
				fptr = calloc(1, sizeof(*fptr));
			/* Opening File */
				fptr = fopen(path_zhemo_achit, "r");

				if (fptr == NULL) {
				    fprintf(stderr,"ERROR: Cannot open file - %s.\n", path_zhemo_achit);
				    report(casename,0);
				    exit(EXIT_FAILURE);
				}
				else {
				    printf("  File opened - %s.\n", path_zhemo_achit);
				}	
			/* Read first lines inside the zhempo file */
				char test[50];
				str = edit_endline_character(line, buffer, fptr);
				sscanf(str, "%s %s",test,name_opensurf);	
			/* free(line); */
	 			fclose(fptr);	

	// copy & unzip the open surface file into /dagon1/achitsaz/FEBio/casename :
	 	// build a path for opensurf in the cebral directory on dagon1:
			strcpy(path_opensurf,pathdb);
			strcat(path_opensurf,mdl);
			strcat(path_opensurf,name_opensurf);
			strcat(path_opensurf,format_gz);		
		if (file_exists(path_opensurf)){
				printf("the name of the open surface file for case %s is : %s\n",casename,name_opensurf);	
		// copy the open surface file to the temp directory: 
			char command3 [100] = "cp ";
			strcat(command3,path_opensurf);
			strcat(command3,space);
			strcat(command3,path_achit);
			strcat(command3,csd);
			system(command3);
		// unzip the surface file at the temp directory
			char command4 [100] = "gzip -d ";
			strcat(command4,path_achit);
			strcat(command4,csd);
			strcat(command4,name_opensurf);
			strcat(command4,format_gz);
			system(command4);
				//system("clear");
		}else{
			fprintf(stderr,"ERROR: there is not %s file in the %s\n",name_opensurf,path_opensurf);
			report(casename,0);
			exit(EXIT_FAILURE);
		}
		strcpy(path_opensurf2,path_achit);
		strcat(path_opensurf2,csd);
		strcat(path_opensurf2,name_opensurf);	 

// creating the path of boundary element labels to open in the chePen3D:
	strcpy(path_labels2,path_opensurf2);
	strcat(path_labels2,label);
// 	creating the path of input file in the /dagon1/achitsaz 
	strcpy(path_input2,path_achit);
	strcat(path_input2,csd);
	strcat(path_input2,name_input);
// propering the close surface file in zfem format for FEBio script in the /dagon1/achitsaz/
	char command5[500]="cp ";
	strcat(command5,path_region);
	strcat(command5,slash);
	strcat(command5,casename);
	strcat(command5,fld_format);
	strcat(command5,space);
	strcat(command5,path_achit);
	strcat(command5,csd);
	system(command5);
// propering the material mask [labels] file for FEBio script in the /dagon1/achitsaz/		
	char command6[500]="cp ";
	strcat(command6,path_region);
	strcat(command6,slash);
	strcat(command6,casename);
	strcat(command6,wall_format);
	strcat(command6,space);
	strcat(command6,path_achit);
	strcat(command6,csd);
	system(command6);
// propering the material mask [labels] file for FEBio script in the /dagon1/achitsaz/		
	char command7[500]="cp ";
	strcat(command7,path_region);
	strcat(command7,slash);
	strcat(command7,casename);
	strcat(command7,aneu_format);
	strcat(command7,space);
	strcat(command7,path_achit);
	strcat(command7,csd);
	system(command7);
// building path done:
	*path_opensurf_achit=path_opensurf2;
	*path_labels=path_labels2;
	*path_input=path_input2;	
}
void clustering_hole(int nelem,int *open, int *esure, int *elems,double	*ptxyz, int **boundary1,double **centers1, int *num_bound1){

	// define paramteres
	int *boundary,num_bound;
	double *centers;

	int ele,element,edge;
	int e;
	int p1,p2;
	int sum=0;
	int i,j,ii,pre_ele_q,iii;
	int num=0;
	int index;
	int *q;
	double cx,cy,cz;
	//allocate memory
		// calculate number of element next to the holes
		for (ele = 0; ele < nelem; ele++){
			if(open[ele]==1){
				sum=sum+1;
			}
		}
	boundary=calloc(4*sum,sizeof(*(boundary)));
	centers=calloc(3*50,sizeof(*(centers))); // Maximum nr of holes in the surface should be bellow than 50
	q=calloc(sum,sizeof(*(q)));
		num=0;	
		for (ele = 0; ele < nelem; ele++){
			if(open[ele]==1){
				// find points on boundaries:
					for(edge=0;edge<3;edge++){
						if(esure[3*ele+edge]==-1){
							if(edge==0){
								p1=elems[3*ele];
								p2=elems[3*ele+1];
							}
							if(edge==1){
								p1=elems[3*ele+1];
								p2=elems[3*ele+2];
								
							}
							if(edge==2){
								p1=elems[3*ele];
								p2=elems[3*ele+2];
								
							}

						}
					}
					boundary[4*num]=ele;
					boundary[4*num+1]=p1;
					boundary[4*num+2]=p2;
					num+=1;	
			}
		}
	index=1;
	
	for (i = 0; i < sum; i++){
	//i=829;
		if(boundary[4*i+3]==0){
			//printf("number of boundary :%d\n",index);
			boundary[4*i+3]=index;
			num=0;
			q[num]=boundary[4*i+1];// put point id in quary
			num+=1;
			//printf("%d\n",boundary[4*i]);
			pre_ele_q=i;
			// build quary:
				for (j = 0; j < num; j++){

					// search which element have p1:
					//printf("%d\n",num);
					for (ii = 0; ii < sum; ii++){
						if(ii!=pre_ele_q && boundary[4*ii+3]==0){

							if(boundary[4*ii+1]==q[j]){
								boundary[4*ii+3]=index;
								q[num]=boundary[4*ii+2];// put another point id in quary
								//printf("%d\t%d\n",ii,boundary[4*ii]);
								num+=1;
								pre_ele_q=ii;
							}
							if(boundary[4*ii+2]==q[j]){
								boundary[4*ii+3]=index;
								q[num]=boundary[4*ii+1];// put another point id in quary
								//printf("%d\t%d\n",ii,boundary[4*ii]);
								num+=1;
								pre_ele_q=ii;

							}


						}


					}

				}

			// calculate the center:
			cx=0;
			cy=0;
			cz=0; 
				for(iii=0;iii<num;iii++){
					cx=cx+ptxyz[3*(q[iii]-1)];
					cy=cy+ptxyz[3*(q[iii]-1)+1];
					cz=cz+ptxyz[3*(q[iii]-1)+2];
					// if(index==1)
					// printf("%lf\t%lf\t%lf\n",ptxyz[3*(q[iii]-1)],ptxyz[3*(q[iii]-1)+1],ptxyz[3*(q[iii]-1)+2]);
				}
				centers[3*index]=cx/num;
				centers[3*index+1]=cy/num;
				centers[3*index+2]=cz/num;
				printf("number of element around this hole:\t%d\n",num);
				printf("center hole %d is x:\t%lf\ty:\t%lf\tz:\t%lf\n",index,cx/num,cy/num,cz/num);


			//update index:
				num_bound=index;
				index+=1;	

		}

	}
//update open:
	for (ele=0;ele<sum;ele++) open[boundary[4*ele]]=boundary[4*ele+3];

// done:
*boundary1=boundary;
*centers1=centers;
*num_bound1=num_bound;

}
void write_input(int num_bound, double *centers, char *filename, char const	*casename){
	// define paramteres
		int b,edge;

	/* Opening labels File*/
		FILE *fptr;
		fptr = calloc(1, sizeof(*fptr));
		fptr = fopen(filename, "w");

		if(fptr == NULL) {
		    fprintf(stderr,"ERROR: Cannot open file - %s.\n", filename);
		    report(casename,0);
		    exit(EXIT_FAILURE);
		}
		else {
		    printf("File opened - %s.\n", filename);
		}


		fprintf(fptr,"#This is a input file for the structural analysis by FEBio\n\n");
		fprintf(fptr,"Casename\n%s\n\n",casename);
		fprintf(fptr,"Labels\n#red\t#yellow\t#white\t#cyan\t#rupture\t#remain\t#boundary\t#transition\t#aneurysm\n");
		fprintf(fptr,"%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n\n",labl_red,labl_yel,labl_whi,labl_cya,labl_rup,labl_rem,labl_bou,labl_tra,labl_ane);
		fprintf(fptr,"Young\t [MPa]\n#red\t#yellow\t#white\t#aneurysm\t#remain\n");
		fprintf(fptr,"%lf\t%lf\t%lf\t%lf\t%lf\n\n",y_red,y_yel,y_whi,y_ane,y_rem);
		fprintf(fptr,"Thickness\t[m]\n#red\t#yellow\t#white\t#aneurysm\t#remain\n");
		fprintf(fptr,"%lf\t%lf\t%lf\t%lf\t%lf\n\n",t_red,t_yel,t_whi,t_ane,t_rem);
		fprintf(fptr,"Density\t[Kg/m^3]\n%lf\n\n",dens);
		fprintf(fptr,"Poisson Number\n%lf\n\n",pois);
		fprintf(fptr,"Pressure\t[MPa]\n#mean pressure (calc pre_stress)\t#ultimate pressure\n%lf\t%lf\n\n",pre_pres,ulti_pres);
		fprintf(fptr,"Centers\t[cm]\n");
		fprintf(fptr,"%d\n",num_bound);
		fprintf(fptr,"#X:\t#Y:\t#Z:\n");
		for(b=1;b<=num_bound;b++) {
			for(edge=0;edge<3;edge++) fprintf(fptr,"%lf\t",centers[3*b+edge]);
				fprintf(fptr,"\n");
		}
}
void write_labels(char const *casename,int nelem, int *open, char *path){

	printf("%s\n",path);

	char filename[500];
	strcpy(filename,path);

	// define paramteres
	int ele;
	printf("%s\n",filename);
	/* Opening labels File*/
		FILE *fptr;
		fptr = calloc(1, sizeof(*fptr));
		fptr = fopen(filename, "w");

		if(fptr == NULL) {
		    fprintf(stderr,"ERROR: Cannot open file - %s.\n", filename);
		    report(casename,0);
		    exit(EXIT_FAILURE);
		}
		else {
		    printf("File opened - %s.\n", filename);
		}

		for(ele=0;ele<nelem;ele++){
				fprintf(fptr,"%d\n",open[ele]);
		}
}