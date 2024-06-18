#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mystructs.h"
#include "common.h"
#include "febiofuncs.h"
#include "myfuncs.h"


int main(int argc, char const *argv[])
{
// reading the argument of main function 
	strcpy(filename, "a06161.1");
	//strcpy(filename,"agh075.1");
	//strcpy(filename, "agh088.1");
	//strcpy(filename, "agh100.1");    
// allocate memory for M1 mesh struct    
    mesh *M1 = (mesh *)malloc(sizeof(mesh));
    if (M1 == NULL) {
        fprintf(stderr, "Memory allocation failed for M1 pointer\n");
        exit(EXIT_FAILURE);
    }
    input *inp = (input *)malloc(sizeof(input));
    if (inp == NULL) {
        fprintf(stderr, "Memory allocation failed for inp pointer\n");
        exit(EXIT_FAILURE);
    }
// input variable : 	
    CHECK_ERROR(rinputf(M1,inp));
// make important directories: 
    CHECK_ERROR(mkdirs());   //printf("run: %s\ndata: %s\n",rundir,datadir);
    CHECK_ERROR(datafiles());
        //printf("path1: %s\npath2: %s\npath3: %s\n",datafilepath[0],datafilepath[1],datafilepath[2]);
// reading domain parameters for unloaded geometry from .FLDS.ZFEM file // 
	CHECK_ERROR(read_zfem(datafilepath[0],&M1->npoin,&M1->nelem,&M1->ptxyz,&M1->elems)); 
// reading wall charectristics [colored fields] from .wall file//
// label : <red=1, yellow=2, white=7, cyan=0, rupture=0, remain=0>
    CHECK_ERROR(read_wallmask(datafilepath[1],M1,inp,&M1->Melem));
        // for (int i=0;i<10;i++) printf("%d\n",M1->Melem[i]);  
// reading regional mask [domme=16 body=8 neck=4 parental=1 distal=2 another e.g. aneu2=0]   
    CHECK_ERROR(read_regionmask(datafilepath[2],M1,inp,&M1->relems,&M1->rpts));
        // for (int i=0;i<10;i++) printf("%d\n",M1->relems[i]);
        // for (int i=0;i<10;i++) printf("%d\n",M1->rpts[i]);
// conver tri3 mesh other type of surface mesh (tri6; quad4; quad8; quad9)  
    // allocate memory for M2 mesh struct    
        mesh *M2 = (mesh *)malloc(sizeof(mesh));
        if (M2 == NULL) {
            fprintf(stderr, "Memory allocation failed for M2 pointer\n");
            exit(EXIT_FAILURE);
        }  
    CHECK_ERROR(ConverMesh(M1,M2,tri3_to_tri6));
// material propertices 
    CHECK_ERROR(calcthick(M2,inp));
    //for (int i=0;i<10;i++) printf("%lf\n",M1->t[i]);
        
    // check the mask after converting :
        char *fieldname1 = "Melems";
        int colo=1;
        char *fieldname2 = "relems"; 
        char *fieldname3 = "thickness"; 
        FunctionWithArgs prtelefield []= {
            { fieldname1,colo,M2->nelem, M2->Melem,SCA_int_VTK},
            { fieldname2,colo,M2->nelem, M2->relems,SCA_int_VTK},
        };
        size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
        FunctionWithArgs prtpntfield []={
            { fieldname3,colo,M2->npoin, M2->t,SCA_double_VTK}
        };
        size_t countpnt = sizeof(prtpntfield) / sizeof(prtpntfield[0]);
        CHECK_ERROR(SaveVTK(rundir,"test",0,M2,tri6funcVTK,prtelefield,countele,prtpntfield,countpnt));

            
    
    return 0;
}