// this code write the input file of the febio  

// include library:
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include <stdbool.h>

// include mylibrary:

#include "mylib.h"


// 	Information of mesh file : 
	int npoin;		//# of points
    int nelem;		//# of elements
 	double *ptxyz;	// cordination of points in mesh file, for triangular mesh : p#x p#y
 	int    *elems;	// conectivity of each element

 	int 	*esure;	// neighbor	elements surround each element
 	int 	*esurp;	// neighbor points surround each point
 	int     *esurp_pointer; // pointer of neighbor points surround each point 
 	int 	*open; // element adjacent hole equal 1 in this array

 	int    num_bound;    // number of inlet/outlet holes
	int    *boundary;     // boundary element according to the order of holes start from 10 index .... 
	double   *centers;	


void help(void){
	printf(" this script just cread files required for write the feb file in the specific directory and write the input data for each case @input.txt\n");
	printf("-required argument:\t 1\n-option:\t--no option--\n-input format:\t hospital_name#.# @ dagon1/jcebral/region/R01/wall/\n");
	printf("-example:\t ./inputmkr agh006.1\n\n ");
	printf("-developer : chitsaz.sci@gmail.com\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]){
	system("rm -rf ../temp/*");
	char helpsc [10] ="help"; 
//find the database in the dagon1 and make directory in the dagon for this new case
		if (argc==1) {
			printf("ERROR: there is problem in specifying database and casename correctly\n");
			exit(EXIT_FAILURE);
		}else{
			
			if (!strcmp(argv[1],helpsc)) help();
			makedir(argv[1]);
		}
	//system("clear");

	 // read the FEBio_GeneralPara.txt 	
		read_Generalinput();  	


	// the name & directory of filenames
		char const *casename=argv[1];
		// build the path of open surface file in the temp directory:
			char *path_opensurf_temp;
			char *path_labels;
			char *path_input;
			build_path(casename,&path_opensurf_temp,&path_labels,&path_input);
	// build required data structures
		// read points and connectivity of the open surface :
			printf("%s\n",path_labels);
			read_zfem(path_opensurf_temp,&npoin, &nelem, &ptxyz, &elems);
			
		// finding neighbor elements surround each point :
			save_esurp(npoin,nelem,elems,&esurp,&esurp_pointer);
		// finding neighbor	elements surround each element :
			save_esure(npoin,nelem,elems,esurp_pointer,esurp,&esure,&open);
			//write_labels(mesh,path_labels);
	// clustering the element of each hole 
		clustering_hole(nelem,open, esure, elems,ptxyz, &boundary,&centers,&num_bound);

		write_labels(nelem,open,path_labels);

		write_input(num_bound,centers,path_input,casename);				

	return 0;
}


