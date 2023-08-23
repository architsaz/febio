// this code build input file according to the febio4
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdbool.h>
#include <time.h>
#define PI 3.142857
#define dimension  3 // dimension of domain 

// my library:
#include "mylib.h"


// declare global parameters 
	//information of MESH
	int npoin;		//# of points
	int nelem;		//# of elements
	double *ptxyz;	// cordination of points in mesh file, for triangular mesh : p#x p#y
	int    *elems;	// conectivity of each element
	int 	*esure;	// neighbor	elements surround each element
	int 	*esurp;	// neighbor points surround each point
	int     *esurp_pointer; // pointer of neighbor points surround each point 
	int 	*open; // element adjacent hole equal 1 in this array
	
	//info mask for region
	int *region_id;  // label of each region for each element
	int *region_idp;	// label of each region for each point

	//info material color label: 
	int    *Melem;  // labels of matrial properties for each element
	int    *Mpoint;    // labels of material properties for each point
	
	int    *anelem;  // labels of aneurysm for each element 

	double *E_ele;		// the Young Modulus for each element before appling Gaussian filter 
	double *E_fele; // the Young Modulus for each element after appling Gaussian filter
	double *E_nod; // the Young Modulus for each point after appling Gaussian filter	 	
	double *t_ele;		// the thickness for each element before appling Gaussian filter
	double *t_fele;		// the thickness for each element after appling Gaussian filter 
	double *t_nod;		// the thickness for each point after appling Gaussian filter

	double *stress; // output result of febiols 
	double *stress_point; // output result of febio
	double *strain;
	double *updated_gstrain;
	double *strain_point;
	double *disp;

	


// pressure ( define @ input.txt)
extern double pre_pres;
extern double ulti_pres;	

// control solver : 
extern int change_solver;

void help(void){
	printf("---->this script just developed for the febio3 and febio4 solver<----\n\n\n");
	printf("-required argument:\t 2\n-option:\t<casename> <febio version>\n-casename format:\t hospital_name#.# @ dagon1/jcebral/region/R01/wall/\n");
	printf("-example:\t ./febmkr agh006.1 febio3\n\n ");
	printf("->developer: chitsaz.sci@gmail.com\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv){

	//system("rm -rf ../../febmkr_report.txt");
	system("rm -rf temp/*");
	char helpsc [10] ="help"; 
	char *path_casedir,*path_input,*path_label,*path_mesh, *path_feb;
	char path_casedir2[500],path_input2[500],path_label2[500],path_mesh2[500];

	int ele,pt;
	int point[3]={0,0,0};
	double pres_gradual; // in [MPa]



	if (!strcmp(argv[1],helpsc)) help();
	/* check the availbelity in the dataset */
	    if (argc==2) {
	    	fprintf(stderr,"ERROR: there is problem in specifying database and casename correctly\n");
	    	report(argv[1],0);
	    	exit(EXIT_FAILURE);
	  	}else{
	  		// find the database in the dagon1 and make directory in the dagon for this new case
	  		checkdir(argv[1],&path_casedir,&path_mesh,&path_label,&path_input);
	  	}
	  	//printf("%s\n%s\n%s\n",path_mesh,path_label,path_input);
	  	// this line just for storage directions and avoid the bug of sending direction string to the functions 
	  	strcpy(path_mesh2,path_mesh);strcpy(path_label2,path_label);strcpy(path_input2,path_input); 
	  	//printf("%s\n%s\n%s\n%s\n",path_casedir,path_mesh2,path_label2,path_input2);

	/* Reading the surface file and Store Node and Elements information */
	  	
		read_zfem(argv[1],path_mesh2,&npoin,&nelem,&ptxyz,&elems);
		

	/* Reading the regional mask (for understanding the parent arteries and neck/domme/...) */
	read_regionmask(argv[1],nelem,npoin,elems,&region_id,&region_idp);
		// to check the mask:
		write_zfem_1intfield(argv[1],nelem,npoin,elems,ptxyz,region_idp);	

	/* Reading the input file : read file "input.txt" */
	read_input(argv[1],path_input2);

	/* Reading the label file [color ID of material] : read file "*.zfem.labels" */
	read_Mlable(argv[1],path_label2,nelem,elems,&Melem);

	// Reading the label of aneurysm "*.aneu.0.0": [0 for parent arteries 1 for aneurysm]
	// update the Melem for the aneurysm area which not collored and give the id_aneu to this element 
	read_aneulable(argv[1],nelem,&anelem, Melem);	

		// just for check material label 
		// build Mpoint for the zfem file format
		Mpoint=calloc(npoin,sizeof(*(Mpoint)));

		for (ele=0;ele<nelem;ele++){
		point[0]=elems[3*ele];
		point[1]=elems[3*ele+1];
		point[2]=elems[3*ele+2];
		for (pt = 0; pt < 3; pt++)  Mpoint[point[pt]-1]=Melem[ele];
		}
		//write_zfem_1intfield(argv[1],nelem,npoin,elems,ptxyz,Mpoint);

	/* finding neighbor elements surround each point : */
	save_esurp(npoin,nelem,elems,&esurp,&esurp_pointer);
	// finding neighbor	elements surround each element :
	save_esure(npoin,nelem,elems,esurp_pointer,esurp,&esure,&open);

	/* Matrial information for each element and grids and applied guassian filter */ 
	// (tip: ele = element data, f=filtered by gussian kernel, E=Young modulus t=thickness)
	int fillter_nloop=10;
	material(nelem,elems,npoin,Melem,esurp,esurp_pointer,fillter_nloop,&E_ele,&E_fele,&E_nod, &t_ele, &t_fele, &t_nod);
		//just for checking the thickness and young modulus;
		//write_zfem_1doubfield(argv[1],nelem,npoin,elems,ptxyz,t_nod);
		//write_zfem_1doubfield(argv[1],nelem,npoin,elems,ptxyz,E_nod);
		//write_zfem_1intfield(argv[1],nelem,npoin,elems,ptxyz,Mpoint);

	// find whole boundary element---->two method: 
	//	1-find inlet/outlet by center of the open surface geometry (It is not accurate method, there is bug in this revision)(this method was comment)
	// -> 2- fixed all element of parent arteries by using the regional mask data (region_id)   		
	//boundary_labeling(nelem,elems,npoin,ptxyz,Melem,Mpoint,esure,region_id);
		// just for check label of boundary
		//write_zfem_1intfield(argv[1],nelem,npoin,elems,ptxyz,Mpoint);

// ------------> in this step the strain gradiant tensor initialized and derived the strain to update the strain gradiant tensor for the next step
	
	/* initialized the updated_strain: */
	updated_gstrain=calloc(6*nelem,sizeof(*(updated_gstrain)));
	strain=calloc(6*nelem,sizeof(*(strain)));
	stress=calloc(6*nelem,sizeof(*(stress)));
	disp=calloc(3*npoin,sizeof(*(disp)));
	for (ele=0;ele<nelem;ele++){
		for (pt = 0; pt < 6; pt++) {
			if(pt==0||pt==1||pt==2) updated_gstrain[6*ele+pt]+=1;
		}
	}
	// prepare options for febio solver 
	char run[500],run_st[500];
	char run_option[100]=" -silent ";
	//char run_option[100]=" ";

	strcpy(run,argv[2]); 	
	strcat(run,run_option);


	/* ITERATIVE METHOD STARTED:*/
	printf(" The FEBio solver start to find gradiant tensor for strain\n");

	printf(" The solver used :\t%s\n",argv[2]);

	char *filename_feb;
	int iter=1;
	int terminate_iter=1;
	char febio_gen3[10]="febio3";char febio_gen4[10]="febio4";
	int CHECK_SOLVER = 0;

	while (terminate_iter==1){
		printf("******************* %d iteration of calculating pre_strain ***************************\n",iter);
		printf("------>CHECK_SOLVER:%d\n",CHECK_SOLVER);

		//value of gradual pre pressure applied  
		if (iter<=19) {
			pres_gradual=(iter)*pre_pres/20;
		}else{
			pres_gradual=pre_pres;
		}
		printf("gradual pressure is : %lf MPa\n",pres_gradual);

		do{

			if (!strcmp(argv[2],febio_gen3)){
			write_feb3_prestain(argv[1],&filename_feb,nelem,elems,npoin,ptxyz,t_fele,E_fele,region_id,updated_gstrain,pres_gradual,iter);
			} else if (!strcmp(argv[2],febio_gen4)){
			
			// find the best time step size for solver 
			if (CHECK_SOLVER==0){
				//the time_stepper in this model is omitted 
				write_feb4_prestain(argv[1],&filename_feb,nelem,elems,npoin,ptxyz,t_fele,E_fele,region_id,updated_gstrain,pres_gradual,iter);
			}else{
				write_feb4_prestain_verold(argv[1],&filename_feb,nelem,elems,npoin,ptxyz,t_fele,E_fele,region_id,updated_gstrain,pres_gradual,iter); 				
			}	

			}else{
				fprintf(stderr,"the solver defined in the argument is not valid\n");
				report(argv[1],0);
				exit(EXIT_FAILURE);
			}

			strcpy(run_st,run);
			strcat(run_st,filename_feb); 

			system(run_st);
			CHECK_SOLVER=check_febio_run(argv[1],iter);

		}while(CHECK_SOLVER==1);	

	/* read strain for guess the gradiant tensor of strain */
		read_logfile_data(argv[1],nelem,npoin,disp,stress,strain,iter);
		//read_data_strain(argv[1],nelem,&strain,iter);

	/* 	checking max of displacment:*/


	/*updating the gradiant tensor of strain:*/
		
		//updateing just by adding the remain strain to the corresponding strain
		for (ele=0;ele<nelem;ele++){
			for (pt = 0; pt < 6; pt++) {
				updated_gstrain[6*ele+pt]+=strain[6*ele+pt];
				if (updated_gstrain[6*ele+pt]<0.00000) updated_gstrain[6*ele+pt]=0; // for correction the value of strain
			}
		}

	printf("--> iteration %d ,  Max_disp: %lf , Max_strain: %lf \n",iter,max_value(disp,3*npoin),max_value(strain,6*nelem));	

		if (terminate_iter==0 || iter>=30) break;
		iter+=1;
	}

// ---------> derive the stress by considering the pre-stress tensor and applied ultimate pressure 

	printf("******************* applied the ultimate pressure by considering the pre_stress tensor  ***************************\n");
	printf("------>CHECK_SOLVER:%d\n",CHECK_SOLVER);

	iter=999;

	do{
		if (!strcmp(argv[2],febio_gen3)){
		write_feb3_prestain(argv[1],&filename_feb,nelem,elems,npoin,ptxyz,t_fele,E_fele,region_id,updated_gstrain,ulti_pres,iter);
		} else{
			// find the best time step size for solver 
			if (change_solver==0){
				//the time_stepper in this model is omitted 
				write_feb4_prestain(argv[1],&filename_feb,nelem,elems,npoin,ptxyz,t_fele,E_fele,region_id,updated_gstrain,pres_gradual,iter);
			}else{
				write_feb4_prestain_verold(argv[1],&filename_feb,nelem,elems,npoin,ptxyz,t_fele,E_fele,region_id,updated_gstrain,pres_gradual,iter); 				
			}
		}
		

		strcpy(run_st,run);
		strcat(run_st,filename_feb); 

		system(run_st);
		check_febio_run(argv[1],iter);

	}while(CHECK_SOLVER==1);	

	read_logfile_data(argv[1],nelem,npoin,disp,stress,strain,iter);


// report the status of run in the txt file:
report(argv[1],1);

return 0;
}