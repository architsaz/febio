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
        fprintf(stderr, "argc is %d\n", argc);
        fprintf(stderr, "ERROR: need more argument\n");
        fprintf(stderr, "<casename> <option> <step> \n\n");
        fprintf(stderr, "available option:\n\nmknjmask\nnocorr\ncorrbynj\nenhance\nhighcurv\n");
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
    else if (!strcmp(argv[2], "highcurv"))
        modifoption = highcurv;
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
        fprintf(stderr, "ERROR: the step assigned for nocoor option should be zero but applied %d\n", step);
        exit(EXIT_FAILURE);
    }
    if (modifoption == highcurv && step != 0)
    {
        fprintf(stderr, "ERROR: the step assigned for highcurv option should be zero but applied %d\n", step);
        exit(EXIT_FAILURE);
    }
    if (modifoption == enhance && step == 0)
    {
        fprintf(stderr, "ERROR: the step assigned for enhance option can not be zero.\n");
        exit(EXIT_FAILURE);
    }
    if (modifoption == corrbynj && step == 0)
    {
        fprintf(stderr, "ERROR: the step assigned for corrbynj option can not be zero.\n");
        exit(EXIT_FAILURE);
    }
    char logname[50], num[10];
    strcpy(logname, febname);
    if (modifoption == mknjmask)
    {
        sprintf(num, "%d", step);
    }
    else
    {
        sprintf(num, "%d", step - 1);
    }
    strcat(logname, "_");
    strcat(logname, num);
    if (modifoption != nocorr && modifoption != highcurv && checkresult(logname) != error)
    {
        fprintf(stderr, "ERROR: the status of %s is not error\n", logname);
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
    // thickness
    CHECK_ERROR(calctrithick(M2, inp));
    CHECK_ERROR(appliedgfilt_ptri6(M1, M2->t, 20));
    // make mask for domain:
    CHECK_ERROR(mkdomain(M1->nelem, M1->esure, M1->relems, inp, &M1->eledomain));
    // material propertices
    double *normang;
    int *norm_mask, *bad_mask, *worst_mask;
    normang = calloc((size_t)M1->nelem, sizeof(*normang));
    norm_mask = calloc((size_t)M1->nelem, sizeof(*norm_mask));
    bad_mask = calloc((size_t)M1->nelem, sizeof(*bad_mask));
    worst_mask = calloc((size_t)M1->nelem, sizeof(*worst_mask));
    CHECK_ERROR(save_esurf(M1->nelem, M1->esure, M1->numf, &M1->esurf, M1->nredge));
    // for (int f=0;f<10;f++) printf("f:%d\tele:%d\tele:%d\n",f,M1->esurf[2*f],M1->esurf[2*f+1]);
    CHECK_ERROR(save_normele(M1->nelem, M1->elems, M1->ptxyz, &M1->normele));
    // for (int ele=0;ele<10;ele++) printf("ele:%d\tpx:%lf\tpy:%lf\tpz:%lf\n",ele,
    // M1->normele[3*ele],M1->normele[3*ele+1],M1->normele[3*ele+2]);
    double dot_product, mag_u, mag_v;
    double u[3] = {0, 0, 0};
    double v[3] = {0, 0, 0};
    for (int f = 0; f < M1->numf; f++)
    {
        for (int i = 0; i < 3; i++)
            u[i] = M1->normele[3 * M1->esurf[2 * f] + i];
        for (int i = 0; i < 3; i++)
            v[i] = M1->normele[3 * M1->esurf[2 * f + 1] + i];
        // Initialized memory
        dot_product = mag_u = mag_v = 0;
        // Calculate dot product and magnitudes
        for (int i = 0; i < 3; i++)
        {
            dot_product += u[i] * v[i];
            mag_u += u[i] * u[i];
            mag_v += v[i] * v[i];
        }
        mag_u = sqrt(mag_u);
        mag_v = sqrt(mag_v);

        // Calculate the cosine of the angle
        if (mag_u == 0)
        {
            fprintf(stderr, "ERROR: magnitude norm ele: %d is zero.\n", M1->esurf[2 * f]);
            exit(EXIT_FAILURE);
        }
        if (mag_v == 0)
        {
            fprintf(stderr, "ERROR: magnitude norm ele: %d is zero.\n", M1->esurf[2 * f + 1]);
            exit(EXIT_FAILURE);
        }
        double anglf = acos(dot_product / (mag_u * mag_v)) * (180.0 / PI);
        if (isnan(anglf))
            anglf = 0;
        normang[M1->esurf[2 * f]] = MAX(normang[M1->esurf[2 * f]], anglf);
        normang[M1->esurf[2 * f + 1]] = MAX(normang[M1->esurf[2 * f + 1]], anglf);
    }
    // make norm/bad/worst mask according to the normal angels
    for (int ele = 0; ele < M1->nelem; ele++)
    {
        if (normang[ele] < inp->norm_ang)
        {
            norm_mask[ele] = 1;
        }
        else if (normang[ele] >= inp->norm_ang && normang[ele] < inp->bad_ang)
        {
            bad_mask[ele] = 1;
        }
        else
        {
            worst_mask[ele] = 1;
        }
    }
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
        CHECK_ERROR(ReadVTK(rundir, "checkinput", step - 1, prtreadfield, countfield));
        M2->young = (double *)field1;
    }
    // modified the Young's Modulous matrix
    if (step != 0)
        printf("**\n**\n* Modifying the Young Modulus with option %s  - step : %d\n**\n**\n", argv[2], step);

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
            fprintf(stderr, "ERROR: Negative Jacobian did not detect in the log file : %s\n", logname);
            exit(EXIT_FAILURE);
        }
        FunctionWithArgs prtelefield2[] = {
            {"NJmask", 1, M2->nelem, NJmask, SCA_double_VTK}};
        size_t countele2 = sizeof(prtelefield2) / sizeof(prtelefield2[0]);
        FunctionWithArgs prtpntfield2[] = {NULL};
        size_t countpnt2 = 0;
        CHECK_ERROR(SaveVTK(rundir, "checkNJmask", step - 1, M2, tri6funcVTK, prtelefield2, countele2, prtpntfield2, countpnt2));
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
    double *highcurvcorr;
    highcurvcorr = calloc((size_t)M1->nelem, sizeof(*highcurvcorr));
    if (modifoption == highcurv)
    {
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            if (bad_mask[ele] == 1 || worst_mask[ele] == 1)
                highcurvcorr[ele] = 1.0;
        }
        CHECK_ERROR(appliedgfilt_etri(M1, highcurvcorr, 5));
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            if (highcurvcorr[ele] > 0.0)
                highcurvcorr[ele] = 1.0;
        }
        CHECK_ERROR(appliedgfilt_etri(M1, highcurvcorr, 5));
        // modified the young modulus using highcurv mask
        for (int ele = 0; ele < M2->nelem; ele++)
            M2->young[ele] += inp->young_highcurv * highcurvcorr[ele];
    }
    // boundary condition
    if (inp->used_BCmask == 1)
        read_BCmask(datafilepath[3], M2, &M2->BCmask);
    CHECK_ERROR(calctripres(M2, M1, inp, worst_mask));
    CHECK_ERROR(calctrifixb(M2, M1, inp, worst_mask));
    if (inp->used_BCmask == 0)
    {
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            if (M1->eledomain[ele] == 1)
            {
                M2->fixbmask[ele] = 0;
                M2->presmask[ele] = 1;
            }
        }
    }
    int *cleanregion;
    // cleanregion = calloc((size_t)M1->nelem, sizeof(*cleanregion));
    CHECK_ERROR(cleanBCmasks(M1, M2->presmask, M2->fixbmask, &cleanregion));
    //  check the mask after converting :
    FunctionWithArgs prtelefield[] =
        {
            {"Melems", 1, M2->nelem, M2->Melem, SCA_int_VTK},
            {"relems", 1, M2->nelem, M2->relems, SCA_int_VTK},
            {"Young_Modulus", 1, M2->nelem, M2->young, SCA_double_VTK},
            {"Press_mask", 1, M2->nelem, M2->presmask, SCA_int_VTK},
            {"fixed_mask", 1, M2->nelem, M2->fixbmask, SCA_int_VTK},
            {"Max_normang", 1, M1->nelem, normang, SCA_double_VTK},
            {"norm_mask", 1, M1->nelem, norm_mask, SCA_int_VTK},
            {"bad_mask", 1, M1->nelem, bad_mask, SCA_int_VTK},
            {"worst_mask", 1, M1->nelem, worst_mask, SCA_int_VTK},
            {"highcurvcorr", 1, M1->nelem, highcurvcorr, SCA_double_VTK},
            {"cleaningregion", 1, M1->nelem, cleanregion, SCA_int_VTK},
            {"domain", 1, M1->nelem, M1->eledomain, SCA_int_VTK},
        };
    size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
    FunctionWithArgs prtpntfield[] = {
        {"thickness", 1, M2->npoin, M2->t, SCA_double_VTK}};
    size_t countpnt = sizeof(prtpntfield) / sizeof(prtpntfield[0]);
    CHECK_ERROR(SaveVTK(rundir, "checkinput", step, M2, tri6funcVTK, prtelefield, countele, prtpntfield, countpnt));
    // writing .feb file
    CHECK_ERROR(febmkr(rundir, febname, step, M2, inp));

    free(norm_mask);
    free(bad_mask);
    free(worst_mask);
    free(highcurvcorr);
    free(cleanregion);
    free(normang);
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
