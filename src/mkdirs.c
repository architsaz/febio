#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int mkdirs(){
    strcpy(rundir,"/runfebio/");
    strcpy(datadir,"/data/");

    strcat(rundir,filename);
    strcat(rundir,slash);
    strcat(datadir,filename);
    strcat(datadir,slash);

    printf("run: %s\ndata: %s\n",rundir,datadir);

    return 0;
}
