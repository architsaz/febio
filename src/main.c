#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "febiofuncs.h"

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

    

}