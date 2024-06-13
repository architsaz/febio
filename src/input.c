#include <string.h>
#include "common.h"
#include "mystructs.h"

int rinputf(mesh *M1,input *inp){
	int e =0;
	// MESH: 
		strcpy(M1->type,"tri");
		M1->nredge=3;
		M1->nrpts=3;
	// Solver 
		strcpy(inp->nonlinear_FE,"full Newton"); // BFGS or  full Newton
		inp->symetric_stiff =0;
	// Neo-Hooken model
		strcpy(inp->Mmodel,"isotropic elastic"); // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
		static double young2 [3]= {10000000,10000000,10000000}; //{red, yellow, white} [dyne/cm^2]
		inp->young=young2;
		inp->young_remain = 10000000;
		inp->pois=0.49; 
		inp->ro=1.101;//[gr/cm^3]		
	// label : <red, yellow, white, cyan, rupture, remain>
		static int label2 [3] = {1,2,7};
		inp->label_num=3;
		inp->label=label2; 
		
	// boundary condition:
		inp->used_maskfile = 0;               //  1 :  use mask --- 0 : used region id 
		static int colorid2 [6] = {0,2,1,4,8,16};   //region { remain(another aneu),diastal,parent,neck,body,dome} 
		inp->colorid_num=6;
		inp->colorid=colorid2;

		static int fix_region2 [6] = {1,1,0,0,0,0}; 	//region { remain(another aneu),diastal,parent,neck,body,dome}
		inp->fix_region=fix_region2;
		inp->fix_region_num=6;
		static int load_region2 [6] = {0,0,1,1,1,1};	//region { remain(another aneu),diastal,parent,neck,body,dome}
		inp->load_region=load_region2;
		inp->load_region_num=6;
	// thickness 
		static double thick_r2[6] ={0.01,0.01,0.01,0.01,0.01,0.01}; // { remain(another aneu),diastal,parent,neck,body,dome}[cm]
		inp->thick_r=thick_r2;
		inp->thick_r_num=6;
		static double thick_l2[3] ={0.005,0.02,0.02}; // {red, yellow, white} [cm]		
		inp->thick_l=thick_l2;
		inp->thick_l_num=3;
	// pre
		//inp->pres=186651;//hypertension [dyne/cm^2]         140 mmHg
		//inp->pres=159987;//systolic [dyne/cm^2]             120 mmHg  
		inp->pres=106658;//diastolic [dyne/cm^2]            80 mmHgquit
		//inp->pres=10000;

		//inp->ultipres=159987;//systolic [dyne/cm^2]             120 mmHg  s
		inp->ultipres=106658;//diastolic [dyne/cm^2]            80 mmHg
		//inp->ultipres=60000;

	return e;
}
int datafiles(){
	int e =0;
	strcpy(datafilepath[0],datadir);
	strcat(datafilepath[0],filename);
	strcat(datafilepath[0],dot);

	strcpy(datafilepath[1],datafilepath[0]);

	strcat(datafilepath[0],"flds.zfem");
	strcat(datafilepath[1],"wall");

	strcpy(datafilepath[2],datadir);
	strcat(datafilepath[2],"labels_srf.zfem");
	return e;
}
