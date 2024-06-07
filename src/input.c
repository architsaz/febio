#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

int rinputf(){

	// MESH: 
		char mesh_type2[50] = "tri6"; // tri3 tri6 quad4 quad8 quad9
		mesh_type=mesh_type2;
	// Solver 
		char nonlinear_FE2 [50] = "full Newton"; // BFGS or  full Newton
		nonlinear_FE=nonlinear_FE2;
		total_iter =8;
		pres_iter = 5;
		symetric_stiff =0;
		used_prev = 0;
		pres_incre = 0.5;	
	
	// uncoupled Moony_Rivlin 2 parameters model
		c1para=1667000; //[dyne/cm^2]
		c2para=0;
		kpara=10000000;
	// Neo-Hooken model
		char Mmodel2 [100] = "isotropic elastic"; // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
		Mmodel=Mmodel2;
		double young2 [3]= {10000000,10000000,10000000}; //{red, yellow, white} [dyne/cm^2]
		young=young2;
		young_remain = 10000000;
		pois=0.49; 
		ro=1.101;//[gr/cm^3]		
	// label : <red, yellow, white, cyan, rupture, remain>
		int label2 [3] = {1,2,7};
		label=label2; 
	// boundary condition:
		used_maskfile = 0;               //  1 :  use mask --- 0 : used region id 
		int colorid2 [6] = {0,2,1,4,8,16};   //region { remain(another aneu),diastal,parent,neck,body,dome} 
		colorid=colorid2;
		int fix_region2 [6] = {1,1,0,0,0,0}; 	//region { remain(another aneu),diastal,parent,neck,body,dome}
		fix_region=fix_region2;
		int load_region2 [6] = {0,0,1,1,1,1};	//region { remain(another aneu),diastal,parent,neck,body,dome}
		load_region=load_region2;
	// analysis region
		int anls_region2 [6] = {0,0,0,1,1,1}; 	//region { remain(another aneu),diastal,parent,neck,body,dome}
		anls_region=anls_region2;
	// thickness 
		double thick_r2[6] ={0.01,0.01,0.01,0.01,0.01,0.01}; // { remain(another aneu),diastal,parent,neck,body,dome}[cm]
		thick_r=thick_r2;
		double thick_l2[3] ={0.005,0.02,0.02}; // {red, yellow, white} [cm]		
		thick_l=thick_l2;
	// pre
	//pres=186651;//hypertension [dyne/cm^2]         140 mmHg
	//pres=159987;//systolic [dyne/cm^2]             120 mmHg  
	pres=106658;//diastolic [dyne/cm^2]            80 mmHgquit
	//pres=10000;

	//ultipres=159987;//systolic [dyne/cm^2]             120 mmHg  s
	ultipres=106658;//diastolic [dyne/cm^2]            80 mmHg
	//ultipres=60000;

	finalpres=159987;//systolic [dyne/cm^2]             120 mmHg  
	//finalpres=106658;//diastolic [dyne/cm^2]            80 mmHg


	char filename2 [100] = "a06161.1";
	//char filename2 [100] = "agh075.1";
	//char filename2 [100] = "agh088.1";
	//char filename2 [100] = "agh100.1";
	filename=filename2;

	return 0;
}
