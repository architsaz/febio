#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

// Lists of macros
	#define PI 3.14159
	#define dimension  3 // dimension of domain 
	#define mesh_edge 3

	#define SQUARE(x) ((x) * (x))
	#define ABS(x) ((x>0) ? (x) : -(x))
	#define MAX(x, y) (((x) > (y)) ? (x) : (y))
	#define MIN(x, y) (((x) < (y)) ? (x) : (y))
// Macro to check function return value and print an error message
	#define CHECK_ERROR(func) do { \
	    int ret = (func); \
	    if (ret != 0) { \
	        fprintf(stderr, "Error: %s failed with error code %d\n", #func, ret); \
	        exit(ret); \
	    } \
	} while (0)

// declare global parameters 
	//information of the closed surface
	extern int npoin,npoin2;		//# of points
	extern int nelem,nelem2;		//# of elements
	extern int    *elems,*elems2;	// conectivity of each element
	extern double *cen;    // center of each element
	extern double *area;   // area of each element 

	extern int 	*esure,*esure_q;	// neighbor	elements surround each element
	extern int 	*esurp,*esurp_q;	// neighbor points surround each point
	extern int     *esurp_pointer,*esurp_pointer_q; // pointer of neighbor points surround each point 
	extern int 	*open,*open_q; // element adjacent boundary equal 1 in this array otherwise 0
	extern int numf,numf_q; // number of face 
    extern int *efid,*efid_q;
    extern int *psurf,*psurf_q;  // ID of points which connected each face

	extern double *ptxyz,*ptxyz2;	// cordination of points in unloaded geometry, for triangular mesh : p#x p#y 
	extern double *pos_unloaded;
	extern double *pos_loaded;
	extern double *disp;

	//info mask for region
	extern int *region_id;  // label of each region for each element
	extern int *region_idp;	// label of each region for each point

	// info fix mask
	extern int *fixmask;

	// info mask for color (label)
	extern int *Melem;	// element-wise lable for color mask

	extern double *gf0,*gf1;      // Deformation Geradient Tensor 

	extern double *st0,*st1;           // Stress tensor
	extern double *eig_st0,*eig_st1;   // Principal stress tensor 

	extern double *t,*t2;

	// MESH: 
	extern	char *mesh_type;
	// Solver 
	extern	char *nonlinear_FE;
	extern	int total_iter,pres_iter,symetric_stiff,used_prev;
	extern	double pres_incre;	
	
	// uncoupled Moony_Rivlin 2 parameters model
	extern	double c1para,c2para,kpara;
	// Neo-Hooken model
	extern	char *Mmodel; // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
	extern	double *young,young_remain,pois,ro;		
	// label :
	extern	int *label; 
	// boundary condition:
	extern	int used_maskfile,*colorid,*fix_region,*load_region;	//region 
	// analysis region
	extern	int *anls_region;//region 
	// thickness 
	extern	double *thick_r,*thick_l; //[cm]		
	
	// pre
	extern double pres,ultipres,finalpres;//[dyne/cm^2]  

	//filename
	extern char *filename;	

#endif // COMMON_H
