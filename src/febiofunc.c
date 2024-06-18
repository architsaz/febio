#include <stdio.h>
#include <stdlib.h>
#include "mystructs.h"

int calcthick (mesh *M, input *inp){
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