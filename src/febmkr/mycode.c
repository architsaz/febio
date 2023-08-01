// include library:
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include <stdbool.h>
#include <time.h>

#include "mylib.h"

void write_log(char *str,char *path){
	char command [500] = "echo ";
	char append_sign [10] = " >> ";
	char quote_sign [10] = " ' ";
	strcat(command,quote_sign);
	strcat(command,str);
	strcat(command,quote_sign);
	strcat(command,append_sign);
	strcat(command,path);
	system(command);
	//printf("%s\n",command);
}
// A utility function to reverse a string
void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}
// Implementation of citoa()
char* citoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}
char *edit_endline_character(char *line, int buffer, FILE *fptr) {

  char *str;
  int len;

  str = fgets(line, buffer, fptr);
  len = strlen(str);
  if(str[len-1] == '\n') str[len-1] = '\0';

  return str;
}
bool file_exists(char const *filename){
    FILE *fp = fopen(filename, "r");
    bool is_exist = false;
    if (fp != NULL)
    {
        is_exist = true;
        fclose(fp); // close the file
    }
    return is_exist;
}
void read_zfem(char *path, int *npoin, int *nelem, double **ptxyz,int **elems) {

	    //printf("input function: %s\n",filename);
	    char filename[500];
	    strcpy(filename,path);

	// defined arrayes and varables
		int npoin1 ,nelem1,*elems1;
		double *ptxyz1;
	
	/* Allocate space to File pointer */
		FILE *fptr;
		fptr = calloc(1, sizeof(*fptr));

	/* Opening File */
		fptr = fopen(filename, "r");

		if (fptr == NULL) {
	    	fprintf(stderr,"ERROR: Cannot open file - %s.\n", filename);
	    	exit(EXIT_FAILURE);
	  	}
	  	else {
	    	printf("  File opened - %s.\n", filename);
	  	}
	  	printf("%s\n",filename);
	  
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
							exit(EXIT_FAILURE);
					    }
					/* Read Number of Points */	    
				      str = edit_endline_character(line, buffer, fptr);     
					    nscan = sscanf(str, "%d",&(npoin1));
					    printf("      Number of Points = %d.\n", npoin1);
						    if (nscan != 1) {
						    	fprintf(stderr,"ERROR: Incorrect number of entries on Number of Points line.\n");
								exit(EXIT_FAILURE);
						    }

					/* Read Coordinates of all points */
					 	 	ptxyz1 = malloc(dimension*npoin1 * sizeof(*(ptxyz1)));				      
					    for (iline = 0; iline < npoin1; iline++) {
				        	str = edit_endline_character(line, buffer, fptr);
							    nscan = sscanf(str, "%lf %lf %lf",
										  &(ptxyz1[dimension*iline + 0]),&(ptxyz1[dimension*iline + 1]),&(ptxyz1[dimension*iline + 2]));
										if (nscan != 3) {
										  fprintf(stderr,"ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline+1);
										  exit(EXIT_FAILURE);
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
						exit(EXIT_FAILURE);
				    }

			  /* Read Connectivity of all elements */
			    	elems1 = malloc(3 * nelem1 * sizeof(*(elems1)));
			      
			    	for (iline = 0; iline < nelem1; iline++) {
			        
			        str = edit_endline_character(line, buffer, fptr);
						  nscan = sscanf(str, "%d %d %d",
								      &(elems1[3*iline]), &(elems1[3*iline + 1]),&(elems1[3*iline + 2]));
						        
								if (nscan != 3) {
								  	fprintf(stderr,"ERROR: Incorrect number of conectinity of elements on line %d th of elements.\n", iline+1);
								  	exit(EXIT_FAILURE);
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

	  /* free(line); */
	  fclose(fptr);
	  *npoin=npoin1;
	  *nelem=nelem1;
	  *elems=elems1;
	  *ptxyz=ptxyz1;
	  
	  /* return */
	  printf("  Exiting function for reading mesh file.\n\n");
}
void save_esurp(int npoin,int nelem,int *elems,int **esurp2,int **esurp_pointer2){
	// define parameter
	int ele,pt,edge,*pointer,*pointer2,*esurp;
	// allocate memory for pointer
		pointer=calloc(npoin+2,sizeof(*(pointer)));
		pointer2=calloc(npoin+2,sizeof(*(pointer2)));
	// find the nr of Elements surround each point
		for(ele=0;ele<nelem;ele++) {
			for(edge=0;edge<dimension;edge++) pointer[elems[dimension*ele+edge]+1]++;
		}

		for(pt=1;pt<=npoin;pt++) pointer[pt+1]+=pointer[pt];
		for(pt=0;pt<=npoin+1;pt++) pointer2[pt]=pointer[pt];
	// allocate memory for the esurp
		esurp=malloc(pointer[npoin+1]*sizeof(*(esurp)));	

	// find elements surround each point
		for(ele=0;ele<nelem;ele++) {
			for (edge=0;edge<dimension;edge++) esurp[pointer[elems[dimension*ele+edge]]++]=ele;
		} 
	
	// done
	free(pointer);
	*esurp2=esurp;
	*esurp_pointer2=pointer2;
}
void save_esure(int npoin,int nelem,int *elems,int *esurp_pointer,int *esurp,int **esue2, int **open2){
	//parameters
	int ele,p1,p2,p3,*nei,*num_nei,*out,*open;

	/* Allocate space to nei pointer */
		nei = calloc(3*nelem, sizeof(*(nei)));
		num_nei=calloc(nelem, sizeof(*(num_nei)));
		open = calloc(nelem, sizeof(*(open)));
	// initializing 
		for (ele = 0; ele < nelem; ele++){
			nei[3*ele]=-1;
			nei[3*ele+1]=-1;
			nei[3*ele+2]=-1;
		}

	for (ele = 0; ele < nelem; ele++){
		p1=elems[3*ele];
		p2=elems[3*ele+1];
		p3=elems[3*ele+2];

		// controller condition
		if (num_nei[ele]==3) continue; 

			if (nei[3*ele]==-1){
				
				out=find_nei_elem3D(esurp_pointer,esurp,num_nei,open,elems,ele,p1,p2);
				
				//printf("1: %d\t%d\n",nei[0],nei[1]);
				if (out[0]!=-9999){
					nei[3*ele]=out[0];
					num_nei[ele]++;
					num_nei[out[0]]++;
					nei[3*out[0]+out[1]]=ele;
				}else{
					nei[3*ele]==-2;
				}
			}
			if (nei[3*ele+1]==-1){
				out=find_nei_elem3D(esurp_pointer,esurp,num_nei,open,elems,ele,p2,p3);
				//printf("2: %d\t%d\n",nei[0],nei[1]);
				if (out[0]!=-9999){
					nei[3*ele+1]=out[0];
					num_nei[ele]++;
					num_nei[out[0]]++;
					nei[3*out[0]+out[1]]=ele;
				}else{
					nei[3*ele+1]==-2;
				}
			}
			if (nei[3*ele+2]==-1){
				out=find_nei_elem3D(esurp_pointer,esurp,num_nei,open,elems,ele,p1,p3);
				//printf("3: %d\t%d\n",nei[0],nei[1]);
				if (out[0]!=-9999){
					nei[3*ele+2]=out[0];
					num_nei[ele]++;
					num_nei[out[0]]++;
					nei[3*out[0]+out[1]]=ele;
				}else{
					nei[3*ele+2]==-2;
				}
			}
			//find the element adjacent to hole
			if (num_nei[ele]<3){
				open[ele]=1;
				//printf("the element %d is near holes\n",ele);
			}
			
	}
	// Done;
	free (num_nei);
	*esue2=nei;
	*open2=open;	
}
int *find_nei_elem3D(int *esurp_pointer,int *esurp,int *num_nei, int *open,int *elems, int ele, int ele_p1, int ele_p2){

	int elemnum,p1,p2,p3,i;
	int *nei;//if nei[0] is -9999 it means that there is problem to find a neighbour element ----------->nei[1] indicate to the number of neighbour in the nei[0]	
	nei=calloc(2, sizeof(*(nei)));
	nei[0]=-9999;
/* find neighbour of ele */
	int *lesps; //list of element around ele_p1 and ele_p2
	int j=0;
	int nr=esurp_pointer[ele_p1+1]+esurp_pointer[ele_p2+1]-esurp_pointer[ele_p1]-esurp_pointer[ele_p2];
	lesps=calloc(nr,sizeof(*lesps));
		for (i = esurp_pointer[ele_p1]; i < esurp_pointer[ele_p1+1]; i++) {
			lesps[j]=esurp[i];
			j++;
		}
		for (i = esurp_pointer[ele_p2]; i < esurp_pointer[ele_p2+1]; i++) {
			lesps[j]=esurp[i];
			j++;
		}
		for (i = 0; i < nr; i++) {

			elemnum = lesps[i];
			if (num_nei[elemnum]<3 && open[elemnum]==0){

				if (elemnum==ele) continue; // checking the same element ID
				
					p1=elems[3*elemnum];
					p2=elems[3*elemnum+1];
					p3=elems[3*elemnum+2];

						if (p1==ele_p1&&p2==ele_p2){
							nei[0]=elemnum;
							nei[1]=0;
							break;
						}
						if(p1==ele_p2&&p2==ele_p1){
							nei[0]=elemnum;
							nei[1]=0;
							break;
						}
						if (p1==ele_p1&&p3==ele_p2){
							nei[0]=elemnum;
							nei[1]=2;
							break;
						}
						if(p1==ele_p2&&p3==ele_p1){
							nei[0]=elemnum;
							nei[1]=2;
							break;
						}
						if (p3==ele_p1&&p2==ele_p2){
							nei[0]=elemnum;
							nei[1]=1;
							break;
						}
						if (p3==ele_p2&&p2==ele_p1){
							nei[0]=elemnum;
							nei[1]=1;
							break;
						}
				
	        	
	        }	
		}

	free(lesps);
	return nei;
}