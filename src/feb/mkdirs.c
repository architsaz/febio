#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globalparafeb.h"
#include "febiofuncs.h"

int mkdirs(void)
{
    strcpy(rundir, "./");
    strcpy(datadir, "../data/");

    return 0;
}
int datafiles(void)
{
    int e = 0;
    strcpy(datafilepath[0], datadir);
    strcat(datafilepath[0], filename);
    strcat(datafilepath[0], dot);

    strcpy(datafilepath[1], datafilepath[0]);

    strcat(datafilepath[0], "flds.zfem");
    strcat(datafilepath[1], "wall");

    strcpy(datafilepath[2], datadir);
    strcat(datafilepath[2], "labels_srf.zfem");

    strcpy(datafilepath[3], datadir);
    strcat(datafilepath[3], "BCmask.zfem.labels");

    strcpy(datafilepath[4], datadir);
    strcat(datafilepath[4], "lc.csv");

    return e;
}