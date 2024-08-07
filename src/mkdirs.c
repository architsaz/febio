#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "febiofuncs.h"

int mkdirs(int step)
{
    strcpy(rundir, "./");
    strcpy(datadir, "../data/");
    char command[100];

    strcpy(command, "rm -r ");
    strcat(command, "*.feb");
    if (step == 0)
        system(command);

    // strcpy(command, "rm -r ");
    // strcat(command, "*.txt");
    // if (step == 0)
    //     system(command);

    strcpy(command, "rm -r ");
    strcat(command, "*.vtk");
    if (step == 0)
        system(command);

    strcpy(command, "rm -r ");
    strcat(command, "*.log");
    if (step == 0)
        system(command);

    strcpy(command, "rm -r ");
    strcat(command, "*.xplt");
    if (step == 0)
        system(command);

    return 0;
}
