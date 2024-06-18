
#ifndef MYSTRUCT_H
#define MYSTRUCT_H
// Define structs 
    typedef struct {
            char type[10]; // tri or quad
            int nredge;    // 3 or 4
            int nrpts;     //<3,6> or <4,8,9>
            int npoin;
            double *ptxyz;
            int nelem;
            int *elems;
            int *esurp;
            int *esurp_ptr;
            int *esure;
            int numf;
            int *fsure;
            int *psurf;
            int *open; // adjacent elements to the open region (inlet/outlet)
            // elemental and pointal mask
            int *Melem; // wall charectristics from .wall file
            int *rpts; // pointal value of regional mask     --> read labels_srf.zfem
            int *relems; // elemental value of regional mask --> approximate
            double *t;  //thickness shell element 
            double *young; // Young modulus for each element [dyne/cm^2]
            int *presmask; // maks of pressure applied for pre-stressed configuration
            int *fixbmask; // maks of fixed element for boundary consition 
    }mesh;
    typedef struct{
        // Solver 
            char nonlinear_FE[50];
            int  symetric_stiff;
        // Neo-Hooken model
            char Mmodel[50]; // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
            double *young,young_remain;
            double pois,ro;		
        // label : <red, yellow, white, cyan, rupture, remain>
            int *label,label_num;
        // boundary condition:
            int *colorid,colorid_num,*fix_region,fix_region_num,*load_region,load_region_num;	//region { remain(another aneu),diastal,parent,neck,body,dome}
        // thickness 
                double *thick_r,*thick_l; // {red, yellow, white} [cm]	
                int thick_r_num,thick_l_num;	            
        // pre
            double pres,ultipres;//[dyne/cm^2]             120 mmHg  
        //filename
            char filename[50];
    }input;

// Define a function pointer type for comparison functions
    typedef int (*compare_func)(void *, void *);
    typedef void ConvertorFunc(mesh *,mesh **); 
    typedef void (*elefieldVTK)(FILE *,char *,int ,int ,void *);  
    typedef void (elemVTK)(FILE *,int ,int *);
    typedef struct {
        char *name;
        int col;
        int nr;   
        void *field;
        elefieldVTK function;
    } FunctionWithArgs;
#endif // MYSTRUCT_H