#include <stdio.h>
#include <stdlib.h>
#include "mystructs.h"

int calctrithick (mesh *M, input *inp){
    int e=0;
    static double *t;
    t = malloc(M->npoin * sizeof(*t));	 
    // applied region mask
        for (int ele =0; ele<M->nelem; ele++){
            for(int k=0;k<inp->colorid_num;k++){
                if (M->relems[ele]==inp->colorid[k]){	
                    for (int p=0;p<M->nrpts;p++) t[(M->elems[M->nrpts*ele+p])-1] = inp->thick_r[k];					
                    break;
                }
            }
        }
    // applied the label{color} mask
        for (int ele =0; ele<M->nelem; ele++){
            for(int k=0;k<inp->label_num;k++){
                if (M->Melem[ele]==inp->label[k]){	
                    for (int p=0;p<M->nrpts;p++) t[(M->elems[M->nrpts*ele+p])-1] = inp->thick_l[k];
                    break;
                }
            }
        }
    M->t=t; 
    printf("* thickness arr calculated by considering regional and material masks\n");   
    return e;    
}
int calctriyoung (mesh *M, input *inp){
    int e=0;
    static double *young;
    young = malloc(M->nelem * sizeof(*young));
    for (int i=0;i<M->nelem;i++) young[i]=inp->young_remain;	 
    // applied the label{color} mask
        for (int ele =0; ele<M->nelem; ele++){
            for(int k=0;k<inp->label_num;k++){
                if (M->Melem[ele]==inp->label[k]){	
                    young[ele] = inp->young[k];
                    break;
                }
            }
        }
    M->young=young; 
    printf("* Young Modulus arr calculated by considering material mask\n");   
    return e;    
}
int calctripres(mesh *M, input *inp){
    int e=0;
    static int *pres;
    pres = calloc(M->nelem, sizeof(*pres));
    // applied the regional mask
        for (int ele =0; ele<M->nelem; ele++){
            for(int k=0;k<inp->load_region_num;k++){
                if (M->relems[ele]==inp->colorid[k]){	
                    pres[ele] = inp->load_region[k];
                    break;
                }
            }
        }
    M->presmask=pres; 
    printf("* A mask of elements is applied by considering both the regional mask and the input file.\n");   
    return e;
}
int calctrifixb(mesh *M, input *inp){
    int e=0;
    static int *fixb;
    fixb = calloc(M->nelem, sizeof(*fixb));
    // applied the regional  mask
        for (int ele =0; ele<M->nelem; ele++){
            for(int k=0;k<inp->fix_region_num;k++){
                if (M->relems[ele]==inp->colorid[k]){	
                    fixb[ele] = inp->fix_region[k];
                    break;
                }
            }
        }
    M->fixbmask=fixb; 
    printf("* A mask of fixed elements by considering both regional mask and the input file.\n");   
    return e;
}