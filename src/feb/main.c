#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mystructs.h"
#include "common.h"
#include "febiofuncs.h"
#include "myfuncs.h"

int main(int argc, char const *argv[])
{
    // parameters:
    char febname[10] = "pres";
    // reading the argument of main function
    if (argc != 4)
    {
        fprintf(stderr,"argc is %d\n",argc);
        fprintf(stderr, "ERROR: need more argument\n");
        fprintf(stderr, "<casename> <mknjmask/nocorr/corrbynj/enhance> <step> \n");
        exit(EXIT_FAILURE);
    }
    strcpy(filename, argv[1]);
    modifyoung modifoption;
    if (!strcmp(argv[2], "corrbynj"))
        modifoption = corrbynj;
    else if (!strcmp(argv[2], "nocorr"))
        modifoption = nocorr;
    else if (!strcmp(argv[2], "enhance"))
        modifoption = enhance;
    else if (!strcmp(argv[2], "mknjmask"))
        modifoption = mknjmask;    
    else
    {
        fprintf(stderr, "ERROR: the option %s used for modifying Young modulus does not match with what supposed for program\n", argv[2]);
        fprintf(stderr, "choose from the this option : nocorr/corrbynj/enhance or mknjmask\n");
        exit(EXIT_FAILURE);
    }
    int step = atoi(argv[3]);
    // Check options
    if (modifoption == nocorr && step != 0)
    {
        fprintf(stderr,"ERROR: the step assigned for nocoor option should be zero but applied %d\n",step);
        exit(EXIT_FAILURE);
    }
    if (modifoption == enhance && step == 0)
    {
        fprintf(stderr,"ERROR: the step assigned for enhance option can not be zero.\n",step);
        exit(EXIT_FAILURE);
    }
    if (modifoption == corrbynj && step == 0)
    {
        fprintf(stderr,"ERROR: the step assigned for corrbynj option can not be zero.\n",step);
        exit(EXIT_FAILURE);
    }
    char logname[50], num[10];
    strcpy(logname, febname);
    if (modifoption == mknjmask){sprintf(num, "%d", step);}
    else{sprintf(num, "%d", step-1);}
    strcat(logname, "_");
    strcat(logname, num);
    if (modifoption != nocorr && checkresult(logname)!=error)
    {
        fprintf(stderr,"ERROR: the status of %s is not error\n",logname);
        exit(EXIT_FAILURE);
    }
    // allocate memory for M1 mesh struct
    mesh *M1 = (mesh *)malloc(sizeof(mesh));
    if (M1 == NULL)
    {
        fprintf(stderr, "Memory allocation failed for M1 pointer\n");
        exit(EXIT_FAILURE);
    }
    input *inp = (input *)malloc(sizeof(input));
    if (inp == NULL)
    {
        fprintf(stderr, "Memory allocation failed for inp pointer\n");
        exit(EXIT_FAILURE);
    }
    // make important directories:
    CHECK_ERROR(mkdirs()); // printf("run: %s\ndata: %s\n",rundir,datadir);
    CHECK_ERROR(datafiles());
    // input variable :
    CHECK_ERROR(rinputf(rundir, M1, inp));
    //    printf("path1: %s\npath2: %s\npath3: %s\n",datafilepath[0],datafilepath[1],datafilepath[2]);
    // reading domain parameters for unloaded geometry from .FLDS.ZFEM file //
    CHECK_ERROR(read_zfem(datafilepath[0], &M1->npoin, &M1->nelem, &M1->ptxyz, &M1->elems));
    // reading wall charectristics [colored fields] from .wall file//
    // label : <red=1, yellow=2, white=7, cyan=0, rupture=0, remain=0>
    CHECK_ERROR(read_wallmask(datafilepath[1], M1, inp, &M1->Melem));
    // for (int i=0;i<10;i++) printf("%d\n",M1->Melem[i]);
    // reading regional mask [domme=16 body=8 neck=4 parental=1 distal=2 another e.g. aneu2=0]
    CHECK_ERROR(read_regionmask(datafilepath[2], M1, inp, &M1->relems, &M1->rpts));
    //   for (int i=0;i<10;i++) printf("%d\n",M1->relems[i]);
    // for (int i=0;i<10;i++) printf("%d\n",M1->rpts[i]);
    // conver tri3 mesh other type of surface mesh (tri6; quad4; quad8; quad9)
    // allocate memory for M2 mesh struct
    mesh *M2 = (mesh *)malloc(sizeof(mesh));
    if (M2 == NULL)
    {
        fprintf(stderr, "Memory allocation failed for M2 pointer\n");
        exit(EXIT_FAILURE);
    }
    CHECK_ERROR(ConverMesh(M1, M2, tri3_to_tri6));
    // allocate memory for Negative Jacobian array:
    NJmask = calloc((size_t)M2->nelem, sizeof(*NJmask));
    // check mknjmask option
    if (modifoption == mknjmask)
    {
        CHECK_ERROR(readNJ(step));
        if (NrNj != 0)
        {
            printf("Nr of Negative Jacobian : %d\n", NrNj);
            FunctionWithArgs prtelefield3[] = {{"NJmask", 1, M2->nelem, NJmask, SCA_double_VTK}};
            size_t countele3 = sizeof(prtelefield3) / sizeof(prtelefield3[0]);
            FunctionWithArgs prtpntfield3[] = {NULL};
            size_t countpnt3 = 0;
            CHECK_ERROR(SaveVTK(rundir, "checkNJmask", step, M2, tri6funcVTK, prtelefield3, countele3, prtpntfield3, countpnt3));
        }
        return 0;
    }
    // material propertices
    CHECK_ERROR(calctrithick(M2, inp));
    CHECK_ERROR(appliedgfilt_ptri6(M1, M2->t, 20));
    if (step == 0)
    {
        CHECK_ERROR(calctriyoung(M2, inp));
        CHECK_ERROR(appliedgfilt_etri(M1, M2->young, 10));
    }
    else
    {
        static void *field1;
        FunctionWithArgs2 prtreadfield[] = {
            {"Young_Modulus", 1, M2->nelem, &field1, read_VTK_double},
        };
        int countfield = sizeof(prtreadfield) / sizeof(prtreadfield[0]);
        CHECK_ERROR(ReadVTK(rundir, "checkinput", step-1, prtreadfield, countfield));
        M2->young = (double *)field1;
    }
    // boundary condition
    if (inp->used_BCmask == 1)
        read_BCmask(datafilepath[3], M2, &M2->BCmask);
    CHECK_ERROR(calctripres(M2, M1, inp));
    CHECK_ERROR(calctrifixb(M2, inp));
    // create feb file
    if (step!=0)printf("**\n**\n* Modifying the Young Modulus with option %s  - step : %d\n**\n**\n", argv[2], step);
    
    if (modifoption == corrbynj)
    {
        // initialized NJmask to avoid the over-correcting on inappropriate region
        for (int i = 0; i < M2->nelem; i++)
            NJmask[i] = 0;
        CHECK_ERROR(readNJ(step - 1));
        CHECK_ERROR(appliedgfilt_etri(M1, NJmask, 10));
        printf("Nr of Negative Jacobian : %d\n", NrNj);
        if (NrNj == 0)
        {   
            fprintf(stderr,"ERROR: Negative Jacobian did not detect in the log file : %s\n",logname);
            exit(EXIT_FAILURE);
        }
        FunctionWithArgs prtelefield2[] = {
            {"NJmask", 1, M2->nelem, NJmask, SCA_double_VTK}};
        size_t countele2 = sizeof(prtelefield2) / sizeof(prtelefield2[0]);
        FunctionWithArgs prtpntfield2[] = {NULL};
        size_t countpnt2 = 0;
        CHECK_ERROR(SaveVTK(rundir, "checkNJmask", step-1, M2, tri6funcVTK, prtelefield2, countele2, prtpntfield2, countpnt2));
        // modified the young modulus using Njmask
        for (int ele = 0; ele < M2->nelem; ele++)
            M2->young[ele] += inp->NJyoung * NJmask[ele];
    }
    if (modifoption == enhance)
    {
        // modified the young modulus just by increasing young modulus
        for (int ele = 0; ele < M2->nelem; ele++)
            M2->young[ele] += inp->incyoung;
    }
    // check the mask after converting :
    FunctionWithArgs prtelefield[] = {
        {"Melems", 1, M2->nelem, M2->Melem, SCA_int_VTK},
        {"relems", 1, M2->nelem, M2->relems, SCA_int_VTK},
        {"Young_Modulus", 1, M2->nelem, M2->young, SCA_double_VTK},
        {"Press_mask", 1, M2->nelem, M2->presmask, SCA_int_VTK},
        {"fixed_mask", 1, M2->nelem, M2->fixbmask, SCA_int_VTK},
    };
    size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
    FunctionWithArgs prtpntfield[] = {
        {"thickness", 1, M2->npoin, M2->t, SCA_double_VTK}};
    size_t countpnt = sizeof(prtpntfield) / sizeof(prtpntfield[0]);
    CHECK_ERROR(SaveVTK(rundir, "checkinput", step, M2, tri6funcVTK, prtelefield, countele, prtpntfield, countpnt));

    CHECK_ERROR(febmkr(rundir, febname, step, M2, inp));

    free(M2->elems);
    free(M2->ptxyz);
    free(M1->relems), free(M1->rpts);
    free(M1->Melem);
    free(M1->elems);
    free(M1->ptxyz);
    free(M1->esure);
    free(M1->psurf);
    free(M1->esurp);
    free(M2->young);
    free(M2->t);
    free(M2->presmask);
    free(M2->fixbmask);
    free(M1);
    free(M2);
    free(inp);
    fflush(stdout);
    return 0;
}
