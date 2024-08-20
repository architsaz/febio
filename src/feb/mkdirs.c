#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "febiofuncs.h"

int mkdirs(void)
{
    strcpy(rundir, "./");
    strcpy(datadir, "../data/");

    return 0;
}
