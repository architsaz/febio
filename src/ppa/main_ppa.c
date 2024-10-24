#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mystructs.h"
#include "common.h"
#include "myfuncs.h"
#include "febiofuncs.h"
#include "globalparappa.h"

int files(void);
int dirs(void);

int main(int argc, char const **argv)
{
    // Check if any arguments are passed
    if (argc < 4)
    {
        fprintf(stderr, "argc is %d\n", argc);
        fprintf(stderr, "ERROR: Follow the correct way of writting arguments\n");
        fprintf(stderr, "As a default need one study and one casename.\n");
        fprintf(stderr, "Usage: %s [--case|-c] [--study|-s] [--iteration|-i] [--time|-t] ... [<arguments>]\n", argv[0]);
        fprintf(stderr, "exp 1 case:            -c a06161.1 -s msa.1 (optional : -i 0 -t end_step1)\n\n");
        fprintf(stderr, "exp 2 compare 2 cases: -c a06161.1 -s msa.1 msa.2 (optional : -i 0 0 -t end_step1 end_step1 )\n\n");
        fprintf(stderr, "* Default need one study and one casename.\n");
        exit(EXIT_FAILURE);
    }

    // Loop through all arguments starting from argv[1]
    for (int i = 1; i < argc; i++)
    {
        // Check for long options or shortcut versions
        if (strcmp(argv[i], "--case") == 0 || strcmp(argv[i], "-c") == 0)
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("Argument for case option: %s\n", argv[i + 1]);
                strcpy(past_filename, argv[i + 1]);
                i++; // Skip the next argument
            }
        }
        else if (strcmp(argv[i], "--study") == 0 || strcmp(argv[i], "-s") == 0)
        {

            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("Argument for study option: %s\n", argv[i + 1]);
                strcpy(study, argv[i + 1]);
                num_study++;
                i++; // Skip the next argument
            }
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("Argument for study option: %s\n", argv[i + 1]);
                strcpy(study2, argv[i + 1]);
                num_study++;
                i++; // Skip the next argument
            }
            printf("* %d study(s) imported as argument.\n", num_study);
        }
        else if (strcmp(argv[i], "--time") == 0 || strcmp(argv[i], "-t") == 0)
        {

            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("Argument for time option: %s\n", argv[i + 1]);
                strcpy(readingtime1, argv[i + 1]);
                num_time++;
                i++; // Skip the next argument
            }
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("Argument for time option: %s\n", argv[i + 1]);
                strcpy(readingtime2, argv[i + 1]);
                num_time++;
                i++; // Skip the next argument
            }
        }
        else if (strcmp(argv[i], "--iteration") == 0 || strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("inserted iteration of study 1: %s\n", argv[i + 1]);
                strcpy(iteration, argv[i + 1]);
                num_iteration++;
                i++; // Skip the next argument
            }
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                // printf("inserted for iteration of study 2: %s\n", argv[i + 1]);
                strcpy(iteration2, argv[i + 1]);
                num_iteration++;
                i++; // Skip the next argument
            }
        }
        else
        {
            printf("Unknown option: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    if (num_study == 0)
    {
        fprintf(stderr, "ERROR: please import at least one study as option\n");
        exit(EXIT_FAILURE);
    }
    // control number of iteration and reading time
    if (num_time != 0 && num_time != num_study)
    {
        fprintf(stderr, "ERROR: number of reading time does not correct!\n");
        exit(EXIT_FAILURE);
    }
    if (num_iteration != 0 && num_iteration != num_study)
    {
        fprintf(stderr, "ERROR: number of iteration should match with number of study!\n");
        exit(EXIT_FAILURE);
    }
    // define the reading time
    read_time rt1, rt2;
    rt1 = end_first_step;
    rt2 = end_first_step;

    if (num_time == 1 && num_time == 2)
    {
        if (!strcmp(readingtime1, "1"))
            rt1 = end_first_step;
        if (!strcmp(readingtime1, "2"))
            rt1 = end_second_step;
        if (!strcmp(readingtime1, "max"))
            rt1 = time_max;
    }
    if (num_time == 2)
    {
        if (!strcmp(readingtime2, "1"))
            rt2 = end_first_step;
        if (!strcmp(readingtime2, "2"))
            rt2 = end_second_step;
        if (!strcmp(readingtime2, "max"))
            rt2 = time_max;
    }

    // make path for files
    CHECK_ERROR(dirs());
    CHECK_ERROR(files());
    // printf("- %s\n", past_datafilepath[0]);
    // printf("- %s\n", past_datafilepath[1]);
    // printf("- %s\n", past_datafilepath[2]);
    // printf("- %s\n", past_datafilepath[3]);
    // printf("- %s\n", past_datafilepath[4]);

    // reading domain parameters for unloaded geometry from .FLDS.ZFEM file //
    mesh *M1 = (mesh *)malloc(sizeof(mesh));
    CHECK_ERROR(read_zfem(past_datafilepath[0], &M1->npoin, &M1->nelem, &M1->ptxyz, &M1->elems));
    strcpy(M1->type, "tri");
    M1->nrpts = 3;
    M1->nredge = 3;
    // input variable :
    input *inp = (input *)malloc(sizeof(input));
    if (inp == NULL)
    {
        fprintf(stderr, "Memory allocation failed for inp pointer\n");
        exit(EXIT_FAILURE);
    }
    CHECK_ERROR(rinputf(pst_rundir, M1, inp));
    // reading Aneurysm region mask
    CHECK_ERROR(read_regionmask(past_datafilepath[3], M1, inp, &M1->relems, &M1->rpts));
    // reading wall charectristics [colored fields] from .wall file//
    // label : <red=1, yellow=2, white=7, cyan=0, rupture=0, remain=0>
    CHECK_ERROR(read_wallmask(past_datafilepath[2], M1, inp, &M1->Melem));
    // calc area of ele
    double *area;
    CHECK_ERROR(calc_area_tri3(M1->ptxyz, M1->elems, M1->nelem, &area));
    // calc norm of ele
    CHECK_ERROR(save_normele(M1->nelem, M1->elems, M1->ptxyz, &M1->normele));
    // flip the normal vector to be outward:
    for (int ele = 0; ele < (3 * M1->nelem); ele++)
        M1->normele[ele] = -1 * M1->normele[ele];
    // read pressure mask from vtk file
    static void *field1;
    FunctionWithArgs2 prtreadfield[] = {
        {"Press_mask", 1, M1->nelem, &field1, read_VTK_int},
    };
    int countfield = sizeof(prtreadfield) / sizeof(prtreadfield[0]);
    CHECK_ERROR(ReadVTK(pst_rundir, "checkinput", 0, prtreadfield, countfield));
    M1->presmask = (int *)field1;

    // read stress tensor on log files
    double *st, *st2;
    CHECK_ERROR(readfebiolog(past_datafilepath[1], M1, &st, rt1));
    if (num_study == 2)
        CHECK_ERROR(readfebiolog(past_datafilepath[4], M1, &st2, rt2));
    // find the eigenvectors and eigenvalues of stress tensor
    double *eigenvalue, *eigenvector, *eigenvalue2, *eigenvector2;
    CHECK_ERROR(jacobiMethod(M1->nelem, st, &eigenvalue, &eigenvector));
    if (num_study == 2)
        CHECK_ERROR(jacobiMethod(M1->nelem, st2, &eigenvalue2, &eigenvector2));
    // sorted eigen vectors and eigen values:
    double *sorted_s, *sorted_s2;
    double *sorted_v, *sorted_v2;
    CHECK_ERROR(sortedsv(M1, eigenvalue, eigenvector, &sorted_s, &sorted_v));
    if (num_study == 2)
        CHECK_ERROR(sortedsv(M1, eigenvalue2, eigenvector2, &sorted_s2, &sorted_v2));
    if (num_study == 1)
    {
        double *smax1, *smax2, *sn, *vmax1, *vmax2, *vn;
        smax1 = calloc((size_t)M1->nelem, sizeof(*smax1));
        smax2 = calloc((size_t)M1->nelem, sizeof(*smax2));
        sn = calloc((size_t)M1->nelem, sizeof(*sn));
        vmax1 = calloc(3 * (size_t)M1->nelem, sizeof(*vmax1));
        vmax2 = calloc(3 * (size_t)M1->nelem, sizeof(*vmax2));
        vn = calloc(3 * (size_t)M1->nelem, sizeof(*vn));
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            smax1[ele] = sorted_s[3 * ele + 1];
            for (int i = 0; i < 3; i++)
                vmax1[3 * ele + i] = sorted_v[9 * ele + 3 + i];
        }

        for (int ele = 0; ele < M1->nelem; ele++)
        {
            smax2[ele] = sorted_s[3 * ele + 2];
            for (int i = 0; i < 3; i++)
                vmax2[3 * ele + i] = sorted_v[9 * ele + 6 + i];
        }

        for (int ele = 0; ele < M1->nelem; ele++)
        {
            sn[ele] = sorted_s[3 * ele + 0];
            for (int i = 0; i < 3; i++)
                vn[3 * ele + i] = sorted_v[9 * ele + 0 + i];
        }

        // find unidirectional or bidirectional stress region mask:
        int *sdir;
        CHECK_ERROR(unibimask(M1, smax1, smax2, &sdir));
        // analysis the unidirectional and biodirectional according to the material mask [red region]

        // write result in VTK format
        FunctionWithArgs prtelefield[] =
            {
                {"SSmax", 1, M1->nelem, smax1, SCA_double_VTK},
                {"SSmin", 1, M1->nelem, smax2, SCA_double_VTK},
                {"Sn", 1, M1->nelem, sn, SCA_double_VTK},
                {"sdir", 1, M1->nelem, sdir, SCA_int_VTK},
                {"Melem", 1, M1->nelem, M1->Melem, SCA_int_VTK},
                {"VSmax", 3, M1->nelem, vmax1, VEC_double_VTK},
                {"VSmin", 3, M1->nelem, vmax2, VEC_double_VTK},
                {"Vn", 3, M1->nelem, vn, VEC_double_VTK},
                {"normele", 3, M1->nelem, M1->normele, VEC_double_VTK},
            };
        size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
        FunctionWithArgs prtpntfield[] = {NULL};
        size_t countpnt = 0;
        CHECK_ERROR(SaveVTK(pstdir, "stress", atoi(iteration), M1, tri3funcVTK, prtelefield, countele, prtpntfield, countpnt));
        CHECK_ERROR(analzs(M1, area, smax1, past_filename, study));
        if (!strcmp(study, "msa.1"))
            CHECK_ERROR(redanals_msa1(M1, sdir, area, past_filename));
        free(sdir);
        free(smax1);
        free(smax2);
        free(vmax1);
        free(vmax2);
    }
    if (num_study == 2)
    {
        double *smax1, *smax2, *vmax1, *vmax2;
        smax1 = calloc((size_t)M1->nelem, sizeof(*smax1));
        smax2 = calloc((size_t)M1->nelem, sizeof(*smax2));
        vmax1 = calloc(3 * (size_t)M1->nelem, sizeof(*vmax1));
        vmax2 = calloc(3 * (size_t)M1->nelem, sizeof(*vmax2));
        // find max eigenvalues and eigenvectors for study 1
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            smax1[ele] = sorted_s[3 * ele + 1];
            for (int i = 0; i < 3; i++)
                vmax1[3 * ele + i] = sorted_v[9 * ele + 3 + i];
        }
        // find max eigenvalues and eigenvectors for study 2
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            smax2[ele] = sorted_s2[3 * ele + 1];
            for (int i = 0; i < 3; i++)
                vmax2[3 * ele + i] = sorted_v2[9 * ele + 3 + i];
        }
        double *comp_smax, *comp_vmax;
        comp_smax = calloc((size_t)M1->nelem, sizeof(*comp_smax));
        comp_vmax = calloc((size_t)M1->nelem, sizeof(*comp_vmax));
        // diff of max of eigenvalues for two studies
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            comp_smax[ele] = smax2[ele] - smax1[ele];
        }
        // clc angle of max eigenvectors for two studies in radians
        for (int ele = 0; ele < M1->nelem; ele++)
        {
            if (M1->presmask[ele] == 0)
                continue;
            double dot_product = 0.0;
            double mag_vmax1 = 0.0;
            double mag_vmax2 = 0.0;

            // Compute dot product and magnitudes
            for (int i = 0; i < 3; i++)
            {
                dot_product += vmax1[3 * ele + i] * vmax2[3 * ele + i];
                mag_vmax1 += vmax1[3 * ele + i] * vmax1[3 * ele + i];
                mag_vmax2 += vmax2[3 * ele + i] * vmax2[3 * ele + i];
            }

            // Normalize the dot product by the magnitudes
            double denom = sqrt(mag_vmax1) * sqrt(mag_vmax2);

            // Avoid division by zero or out-of-range acos input
            double teta = (denom > 1e-10) ? dot_product / denom : 0.0;

            // Clamp teta to the valid range for acos to avoid NaN
            if (teta > 1.0)
                teta = 1.0;
            if (teta < -1.0)
                teta = -1.0;

            comp_vmax[ele] = acos(teta); // Angle in radians
        }
        // write result in VTK format
        FunctionWithArgs prtelefield[] =
            {
                {"smax1", 1, M1->nelem, smax1, SCA_double_VTK},
                {"smax2", 1, M1->nelem, smax2, SCA_double_VTK},
                {"comp_smax", 1, M1->nelem, comp_smax, SCA_double_VTK},
                {"comp_vmax", 1, M1->nelem, comp_vmax, SCA_double_VTK},
                {"vmax1", 3, M1->nelem, vmax1, VEC_double_VTK},
                {"vmax2", 3, M1->nelem, vmax2, VEC_double_VTK},
            };
        size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
        FunctionWithArgs prtpntfield[] = {NULL};
        size_t countpnt = 0;
        CHECK_ERROR(SaveVTK(pstdir, "comp_stress", 0, M1, tri3funcVTK, prtelefield, countele, prtpntfield, countpnt));

        free(comp_smax);
        free(comp_vmax);
        free(smax1);
        free(smax2);
        free(vmax1);
        free(vmax2);
    }
    free(st);
    free(st2);
    free(eigenvalue);
    free(eigenvector);
    free(eigenvalue2);
    free(eigenvector2);
    free(area);
    free(sorted_s);
    free(sorted_s2);
    free(sorted_v);
    free(sorted_v2);
    free(inp);
    free(M1);
    return 0;
}

int files(void)
{
    int e = 0;
    strcpy(past_datafilepath[0], pst_datadir);
    strcat(past_datafilepath[0], past_filename);
    strcat(past_datafilepath[0], ".");
    strcat(past_datafilepath[0], "flds.zfem");

    strcpy(past_datafilepath[1], pst_rundir);
    strcat(past_datafilepath[1], febname);
    strcat(past_datafilepath[1], "_");
    strcat(past_datafilepath[1], iteration);
    strcat(past_datafilepath[1], ".log");

    strcpy(past_datafilepath[2], pst_datadir);
    strcat(past_datafilepath[2], past_filename);
    strcat(past_datafilepath[2], ".");
    strcat(past_datafilepath[2], "wall");

    strcpy(past_datafilepath[3], pst_datadir);
    strcat(past_datafilepath[3], "labels_srf.zfem");

    if (num_study == 2)
    {
        strcpy(past_datafilepath[4], pst_rundir2);
        strcat(past_datafilepath[4], febname);
        strcat(past_datafilepath[4], "_");
        strcat(past_datafilepath[4], iteration2);
        strcat(past_datafilepath[4], ".log");
    }

    return e;
}
int dirs(void)
{
    strcpy(pst_rundir, "../");
    strcat(pst_rundir, study);
    strcat(pst_rundir, "/");
    if (num_study == 2)
    {
        strcpy(pst_rundir2, "../");
        strcat(pst_rundir2, study2);
        strcat(pst_rundir2, "/");
    }
    strcpy(pst_datadir, "../data/");

    return 0;
}
