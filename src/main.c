#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "febiofuncs.h"
#include "myfuncs.h"

int main(int argc, char const *argv[])
{
// input variable : 	
    CHECK_ERROR(rinputf());
// make important directories: 
    CHECK_ERROR(mkdirs());    //printf("run: %s\ndata: %s\n",rundir,datadir);
    CHECK_ERROR(datafiles());
    //printf("path1: %s\npath2: %s\npath3: %s\n",datafilepath[0],datafilepath[1],datafilepath[2]);
// reading domain parameters for unloaded geometry from .FLDS.ZFEM file // 
	CHECK_ERROR(read_zfem(datafilepath[0],&npoin,&nelem,&ptxyz,&elems)); 
// reading wall charectristics [colored fields] from .wall file//
// label : <red=1, yellow=2, white=7, cyan=0, rupture=0, remain=0>
    CHECK_ERROR(read_wallmask(datafilepath[1],&Melem)); 
// reading regional mask [domme=16 body=8 neck=4 parental=1 distal=2 another e.g. aneu2=0]   
    CHECK_ERROR(read_regionmask(datafilepath[2],&region_id,&region_idp));
// find element surround a point
    CHECK_ERROR(save_esurp(npoin,nelem,elems,&esurp,&esurp_pointer,mesh_edge));
    //for (int i=0;i<20;i++) printf("%d \n",esurp_pointer[i]);  
// find element surround an element 
    CHECK_ERROR(save_esure(nelem,elems,esurp_pointer,esurp,&esure,&open,mesh_edge));
    //for (int i=0;i<nelem;i++) printf("ele : %d e1: %d e2 : %d e3: %d\n",i,esure[3*i],esure[3*i+1],esure[3*i+2]);
// find Nr of eadge and given id to adges*/
    CHECK_ERROR(save_fsure(nelem,esure,&efid,&numf,mesh_edge)); 
    // printf (" the number of face : %d \n",numf);
    // for (int i=0; i<nelem ; i++){
    //     printf("ele %d l1: %d l2: %d l3: %d \n",i,efid[3*i],efid[3*i+1],efid[3*i+2]);
    // } 
// find point surround a face*/
    CHECK_ERROR(save_psurf(nelem,numf,elems,esure,&psurf,mesh_edge));
    // for (int i=0; i<numf ; i++){
    //     printf("f %d p1: %d p2: %d \n",i,psurf[2*i],psurf[2*i+1]);
    // }

    
    return 0;
}