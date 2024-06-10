#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int mkdirs(){
    strcpy(rundir,"runfebio/");
    strcpy(datadir,"data/");
    char command[50]="mkdir ";
    strcat(command,rundir);
    system(command);
    strcat(rundir,filename);
    strcat(rundir,slash);
    strcpy(command,"rm -r ");
    strcat(command,rundir);
    system(command);

    strcpy(command,"mkdir ");
    strcat(command,rundir);
    system(command);

    strcat(datadir,filename);
    strcat(datadir,slash);

    return 0;
}
