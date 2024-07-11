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
    // reading the argument of main function
    // strcpy(filename, "a06161.1");
    // strcpy(filename,"agh075.1");
    // strcpy(filename, "agh088.1");
    strcpy(filename, "agh100.1");
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
    // input variable :
    CHECK_ERROR(rinputf(M1, inp));
    // make important directories:
    CHECK_ERROR(mkdirs()); // printf("run: %s\ndata: %s\n",rundir,datadir);
    CHECK_ERROR(datafiles());
    // printf("path1: %s\npath2: %s\npath3: %s\n",datafilepath[0],datafilepath[1],datafilepath[2]);
    // reading domain parameters for unloaded geometry from .FLDS.ZFEM file //
    CHECK_ERROR(read_zfem(datafilepath[0], &M1->npoin, &M1->nelem, &M1->ptxyz, &M1->elems));
    // reading wall charectristics [colored fields] from .wall file//
    // label : <red=1, yellow=2, white=7, cyan=0, rupture=0, remain=0>
    CHECK_ERROR(read_wallmask(datafilepath[1], M1, inp, &M1->Melem));
    // for (int i=0;i<10;i++) printf("%d\n",M1->Melem[i]);
    // reading regional mask [domme=16 body=8 neck=4 parental=1 distal=2 another e.g. aneu2=0]
    CHECK_ERROR(read_regionmask(datafilepath[2], M1, inp, &M1->relems, &M1->rpts));
    // for (int i=0;i<10;i++) printf("%d\n",M1->relems[i]);
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
    // material propertices
    CHECK_ERROR(calctrithick(M2, inp));
    CHECK_ERROR(appliedgfilt_ptri6(M1, M2->t, 20));
    CHECK_ERROR(calctriyoung(M2, inp));
    CHECK_ERROR(appliedgfilt_etri(M1, M2->young, 10));
    CHECK_ERROR(calctripres(M2, inp));
    CHECK_ERROR(calctrifixb(M2, inp));
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
    CHECK_ERROR(SaveVTK(rundir, "checkinput", 0, M2, tri6funcVTK, prtelefield, countele, prtpntfield, countpnt));
    // creat feb file
    int step = 0;
    char febname[10] = "pres";
    CHECK_ERROR(febmkr(rundir, febname, step, M2, inp));
    // run febio solver
    CHECK_ERROR(runfebio(0));
    // printf("error: %d\n",checkresult("pres_0"));
    // check Negative Jacobian:
    NJmask = calloc(M2->nelem, sizeof(NJmask));
    double *org_young;
    org_young = calloc(M2->nelem, sizeof(*org_young));
    char logname[50];
    strcpy(logname, febname);
    strcat(logname, "_0");
    // while (checkresult("pres_0")==1 && step<1){
    while (checkresult(logname) == 1 && step < 10)
    {
        step++;
        printf("**\n**\n* Fixing Negative jacobian detected - step : %d\n**\n**\n", step);
        strcpy(logname, febname);
        char num[10];
        sprintf(num, "%d", step);
        strcat(logname, "_");
        strcat(logname, num);

        // initialized NJmask to avoid the over-correcting on inappropriate region
        for (int i = 0; i < M2->nelem; i++)
            NJmask[i] = 0;

        CHECK_ERROR(readNJ(step - 1));
        CHECK_ERROR(appliedgfilt_etri(M1, NJmask, 10));

        printf("Nr of Negative Jacobian : %d\n", NrNj);
        if (NrNj == 0)
            break;
        FunctionWithArgs prtelefield2[] = {
            {"NJmask", 1, M2->nelem, NJmask, SCA_double_VTK}};
        size_t countele2 = sizeof(prtelefield2) / sizeof(prtelefield2[0]);
        FunctionWithArgs prtpntfield2[] = {};
        size_t countpnt2 = sizeof(prtpntfield2) / sizeof(prtpntfield2[0]);
        CHECK_ERROR(SaveVTK(rundir, "checkNJmask", step, M2, tri6funcVTK, prtelefield2, countele2, prtpntfield2, countpnt2));
        // modified the young modulus using Njmask
        for (int ele = 0; ele < M2->nelem; ele++)
            M2->young[ele] += inp->NJyoung * NJmask[ele];
        CHECK_ERROR(SaveVTK(rundir, "checkinput", step, M2, tri6funcVTK, prtelefield, countele, prtpntfield, countpnt));

        CHECK_ERROR(febmkr(rundir, febname, step, M2, inp));
        CHECK_ERROR(runfebio(step));
    }

    return 0;
}