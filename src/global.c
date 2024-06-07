#include "common.h"

// declare global parameters 
	//information of the closed surface
	int npoin,npoin2;		//# of points
	int nelem,nelem2;		//# of elements
	int    *elems,*elems2;	// conectivity of each element
	double *cen;    // center of each element
	double *area;   // area of each element 

	int 	*esure,*esure_q;	// neighbor	elements surround each element
	int 	*esurp,*esurp_q;	// neighbor points surround each point
	int     *esurp_pointer,*esurp_pointer_q; // pointer of neighbor points surround each point 
	int 	*open,*open_q; // element adjacent boundary equal 1 in this array otherwise 0
	int numf,numf_q; // number of face 
    int *efid,*efid_q;
    int *psurf,*psurf_q;  // ID of points which connected each face

	double *ptxyz,*ptxyz2;	// cordination of points in unloaded geometry, for triangular mesh : p#x p#y 
	double *pos_unloaded;
	double *pos_loaded;
	double *disp;

	//info mask for region
	int *region_id;  // label of each region for each element
	int *region_idp;	// label of each region for each point

	// info fix mask
	int *fixmask;

	// info mask for color (label)
	int *Melem;	// element-wise lable for color mask

	double *gf0,*gf1;      // Deformation Geradient Tensor 

	double *st0,*st1;           // Stress tensor
	double *eig_st0,*eig_st1;   // Principal stress tensor 

	double *t,*t2;

	// MESH: 
		char *mesh_type;
	// Solver 
		char *nonlinear_FE;
		int total_iter,pres_iter,symetric_stiff,used_prev;
		double pres_incre;	
	
	// uncoupled Moony_Rivlin 2 parameters model
		double c1para,c2para,kpara;
	// Neo-Hooken model
		char *Mmodel; // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
		double *young,young_remain,pois,ro;		
	// label : <red, yellow, white, cyan, rupture, remain>
		int *label; 
	// boundary condition:
		int used_maskfile,*colorid,*fix_region,*load_region;	//region { remain(another aneu),diastal,parent,neck,body,dome}
	// analysis region
		int *anls_region;//region { remain(another aneu),diastal,parent,neck,body,dome}
	// thickness 
		double *thick_r,*thick_l; // {red, yellow, white} [cm]		
	
	// pre
	double pres,ultipres,finalpres;//[dyne/cm^2]             120 mmHg  

	//filename
	char *filename;

