#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int mkdirs(int step ){
    strcpy(rundir,"runfebio/");
    strcpy(datadir,"data/");
    char command[50]="mkdir ";
    strcat(command,rundir);
    if (step==0) system(command);
    strcat(rundir,filename);
    strcat(rundir,slash);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.feb");
    if (step==0) system(command);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.txt");
    if (step==0) system(command);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.sh");
    if (step==0) system(command);

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.vtk");
    if (step==0) system(command);    

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.log");
    if (step==0) system(command); 

    strcpy(command,"rm -r ");
    strcat(command,rundir);
    strcat(command,"*.xplt");
    if (step==0) system(command); 

    strcpy(command,"mkdir ");
    strcat(command,rundir);
    if (step==0) system(command);

    strcat(datadir,filename);
    strcat(datadir,slash);

    return 0;
}
