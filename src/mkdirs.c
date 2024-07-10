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
    strcat(command,"*.feb");
    system(command);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.txt");
    system(command);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.sh");
    system(command);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.vtk");
    system(command);    

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.log");
    system(command); 

    strcpy(command,"mkdir ");
    strcat(command,rundir);
    system(command);

    strcat(datadir,filename);
    strcat(datadir,slash);

    return 0;
}
