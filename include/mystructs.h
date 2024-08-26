
#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#define TABLE_SIZE 500
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
            int *esurf;
            double *normele;
            int *open; // adjacent elements to the open region (inlet/outlet)
            // elemental and pointal mask
            int *Melem; // wall charectristics from .wall file
            int *rpts; // pointal value of regional mask     --> read labels_srf.zfem
            int *relems; // elemental value of regional mask --> approximate
            double *t;  //thickness shell element 
            double *young; // Young modulus for each element [dyne/cm^2]
            int *presmask; // mask of pressure applied for pre-stressed configuration
            int *fixbmask; // mask of fixed element for boundary consition 
            int *BCmask;   // mask of Boundary Condition is getting its value from the BCmask.txt file. 
    }mesh;
    typedef struct{
        // Solver 
            char nonlinear_FE[50];
            int  symetric_stiff;
        // curvature mask    
            double norm_ang;
            double young_highcurv;
        // Neo-Hooken model
            char Mmodel[50]; // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
            double *young_r,*young_l,NJyoung,incyoung;
            double pois,ro;	
        // mask status 
            int used_cmask,used_rmask;    	
        // label : <red, yellow, white, cyan, rupture, remain>
            int *label,label_num;
        // boundary condition:
            int *colorid,colorid_num,*fix_region,fix_region_num,*load_region,load_region_num;	//region { remain(another aneu),diastal,parent,neck,body,dome}
            int used_BCmask;      
        // thickness 
                double *thick_r,*thick_l; // {red, yellow, white} [cm]	
                int thick_r_num,thick_l_num;	            
        // pre
            double pres,ultipres;//[dyne/cm^2]             120 mmHg  
        //filename
            char filename[50];
        // logfile 
            int print_st;
    }input;
    // Define the structure for a hash table entry
    typedef struct Entry {
        char *key;
        char *value;
        struct Entry *next;  // For handling collisions using chaining
    } Entry;
    // Define the structure for the hash table
    typedef struct {
        Entry *entries[TABLE_SIZE];
    } HashTable;
// Define enum parameter
    typedef enum {
        corrbynj,
        unify,
        enhance,
        nocorr,
        mknjmask,
        highcurv,
    } modifyoung; 
    typedef enum {
        error,
        normal,
        unknown,
    } runstatus;
// Define function pointers 
    typedef int (*compare_func)(void *, void *);

    typedef void ConvertorFunc(mesh *,mesh **); 

    typedef void (*elefieldVTK)(FILE *,char *,int ,int ,void *);
    typedef void (*readfieldVTK)(FILE *,int ,int ,void **); 
    typedef struct {
        char *name;
        int col;
        int nr;   
        void *field;
        elefieldVTK function;
    } FunctionWithArgs;
    typedef struct {
        char *name;
        int col;
        int nr; 
        void **arr;  
        readfieldVTK function;
    } FunctionWithArgs2; 
    typedef void (elemVTK)(FILE *,int ,int *);      
   
#endif // MYSTRUCT_H