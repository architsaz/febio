#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mystructs.h"
#include "common.h"
#include "febiofuncs.h"
#include "myfuncs.h"
// analysis the red region and color mask for msa.1
int redanals_msa1(mesh *M, int *sdir, double *area, char *casename)
{
    int e = 0;

    double totalred, unidred, bidred, bidperc;
    char Red_exist[10] = "YES";
    totalred = unidred = bidred = bidperc = 0.0;
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->Melem[ele] == 1)
        {
            totalred += area[ele];
            if (sdir[ele] == 3 || sdir[ele] == 1)
                unidred += area[ele];
            if (sdir[ele] == 4 || sdir[ele] == 2)
                bidred += area[ele];
        }
    }
    printf("- area: %lf %lf %lf\n", totalred, unidred, bidred);
    if (totalred > 0.00001)
    {
        bidperc = (bidred / totalred) * 100;
    }
    else
    {
        bidred = 0.0;
        strcpy(Red_exist, "No");
    }
    printf("- Percentage of Red Region being bidirectional : %lf %%\n", bidperc);
    double totalbid, redbid, bidonredprec;
    totalbid = redbid = 0;
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (sdir[ele] == 4)
        {
            if (M->relems[ele] == 4 || M->relems[ele] == 8 || M->relems[ele] == 16)
            {
                totalbid += area[ele];
                if (M->Melem[ele] == 1)
                {
                    redbid += area[ele];
                }
            }
        }
    }
    bidonredprec = (redbid / totalbid) * 100;
    printf("- Percentage of bidiregional region being red region : %lf %%\n", bidonredprec);

    // save Percentage in a table txt
    // arrays of different types and sizes
    char *arr1[2] = {"bid_is_Red", "Red_is_bid"};
    double arr2[2] = {bidonredprec, bidperc};
    char *arr3[2];
    for (int i = 0; i < 2; i++)
    {
        arr3[i] = (char *)malloc(strlen(casename) + 1); // +1 for the null terminator
        if (arr1[i] != NULL)
        {
            strcpy(arr3[i], casename);
        }
        else
        {
            // Handle memory allocation failure
            fprintf(stderr, "ERROR: Memory allocation failed for arr3[%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }
    char *arr4[2];
    for (int i = 0; i < 2; i++)
    {
        arr4[i] = (char *)malloc(strlen(Red_exist) + 1); // +1 for the null terminator
        if (arr1[i] != NULL)
        {
            strcpy(arr4[i], Red_exist);
        }
        else
        {
            // Handle memory allocation failure
            fprintf(stderr, "ERROR: Memory allocation failed for arr4[%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }
    // Number of arrays and their sizes
    int numArrays = 4;
    int sizes[] = {2, 2, 2, 2}; // Sizes of the arrays

    // Array of pointers to the arrays
    void *arrays[] = {
        (void *)arr3,
        (void *)arr4,
        (void *)arr1,
        (void *)arr2,
    };
    // Data types of each array (int, float, char, string)
    DataType types[] = {STRING_TYPE, STRING_TYPE, STRING_TYPE, DOUBLE_TYPE};

    // Array of column headers (names)
    const char *headers[] = {"Casename", "Red_exist", "Parameters", "Percentage %"};

    // Save the arrays to the file with headers
    saveMultipleArraysToFile("redregionanals.txt", numArrays, arrays, sizes, types, headers);

    return e;
}
// analysis the max stress @ color region mask and region mask
int analzs(mesh *M, double *area, double *smax1, char *casename, char *study)
{
    int e = 0;

    int nele_red, nele_yel, nele_wht, nele_press, nele_dom, nele_bod, nele_nek, nele_part, nele_aneu;
    nele_red = nele_yel = nele_wht = nele_press = nele_dom = nele_bod = nele_nek = nele_part = nele_aneu = 0;
    double *smax_red, *smax_yel, *smax_wht, *smax_press, *smax_aneu, *smax_dom, *smax_bod, *smax_nek, *smax_part;
    double *area_red, *area_yel, *area_wht, *area_press, *area_aneu, *area_dom, *area_bod, *area_nek, *area_part;
    // find the size of each domain
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->presmask[ele] == 0)
            continue;
        nele_press++;
        if (M->Melem[ele] == 1)
            nele_red++;
        if (M->Melem[ele] == 2)
            nele_yel++;
        if (M->Melem[ele] == 7)
            nele_wht++;
        if (M->relems[ele] == 16)
            nele_dom++;
        if (M->relems[ele] == 8)
            nele_bod++;
        if (M->relems[ele] == 4)
            nele_nek++;
        if (M->relems[ele] == 1)
            nele_part++;
        if (M->relems[ele] == 4 || M->relems[ele] == 8 || M->relems[ele] == 16)
            nele_aneu++;
    }
    printf("nele_press: %d \n", nele_press);
    printf("nele_red: %d \n", nele_red);
    printf("nele_yel: %d \n", nele_yel);
    printf("nele_wht: %d \n", nele_wht);
    printf("nele_dom: %d \n", nele_dom);
    printf("nele_bod: %d \n", nele_bod);
    printf("nele_nek: %d \n", nele_nek);
    printf("nele_part: %d \n", nele_part);
    printf("nele_aneu: %d \n", nele_aneu);
    smax_red = calloc((size_t)nele_red, sizeof(*smax_red));
    smax_yel = calloc((size_t)nele_yel, sizeof(*smax_yel));
    smax_wht = calloc((size_t)nele_wht, sizeof(*smax_wht));
    smax_aneu = calloc((size_t)nele_aneu, sizeof(*smax_aneu));
    smax_press = calloc((size_t)nele_press, sizeof(*smax_press));
    smax_dom = calloc((size_t)nele_dom, sizeof(*smax_dom));
    smax_bod = calloc((size_t)nele_bod, sizeof(*smax_bod));
    smax_nek = calloc((size_t)nele_nek, sizeof(*smax_nek));
    smax_part = calloc((size_t)nele_part, sizeof(*smax_part));
    area_red = calloc((size_t)nele_red, sizeof(*area_red));
    area_yel = calloc((size_t)nele_yel, sizeof(*area_yel));
    area_wht = calloc((size_t)nele_wht, sizeof(*area_wht));
    area_aneu = calloc((size_t)nele_aneu, sizeof(*area_aneu));
    area_press = calloc((size_t)nele_press, sizeof(*area_press));
    area_dom = calloc((size_t)nele_dom, sizeof(*area_dom));
    area_bod = calloc((size_t)nele_bod, sizeof(*area_bod));
    area_nek = calloc((size_t)nele_nek, sizeof(*area_nek));
    area_part = calloc((size_t)nele_part, sizeof(*area_part));
    nele_red = nele_yel = nele_wht = nele_press = nele_dom = nele_bod = nele_nek = nele_part = nele_aneu = 0;
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->presmask[ele] == 0)
            continue;
        smax_press[nele_press] = fabs(smax1[ele]);
        area_press[nele_press] = area[ele];
        nele_press++;
        if (M->Melem[ele] == 1)
        {
            smax_red[nele_red] = fabs(smax1[ele]);
            area_red[nele_red] = area[ele];
            nele_red++;
        }

        if (M->Melem[ele] == 2)
        {
            smax_yel[nele_yel] = fabs(smax1[ele]);
            area_yel[nele_yel] = area[ele];
            nele_yel++;
        }

        if (M->Melem[ele] == 7)
        {
            smax_wht[nele_wht] = fabs(smax1[ele]);
            area_wht[nele_wht] = area[ele];
            nele_wht++;
        }

        if (M->relems[ele] == 16)
        {
            smax_dom[nele_dom] = fabs(smax1[ele]);
            area_dom[nele_dom] = area[ele];
            nele_dom++;
        }

        if (M->relems[ele] == 8)
        {
            smax_bod[nele_bod] = fabs(smax1[ele]);
            area_bod[nele_bod] = area[ele];
            nele_bod++;
        }

        if (M->relems[ele] == 4)
        {
            smax_nek[nele_nek] = fabs(smax1[ele]);
            area_nek[nele_nek] = area[ele];
            nele_nek++;
        }

        if (M->relems[ele] == 1)
        {
            smax_part[nele_part] = fabs(smax1[ele]);
            area_part[nele_part] = area[ele];
            nele_part++;
        }

        if (M->relems[ele] == 4 || M->relems[ele] == 8 || M->relems[ele] == 16)
        {
            smax_aneu[nele_aneu] = fabs(smax1[ele]);
            area_aneu[nele_aneu] = area[ele];
            nele_aneu++;
        }
    }
    // Calculate statistics
    double *stat_red, *stat_yel, *stat_wht, *stat_aneu, *stat_dom, *stat_bod, *stat_nek, *stat_part, *stat_press;
    double stat_empty[4] = {0, 0, 0, 0};
    if (nele_red != 0)
    {
        mystat(smax_red, nele_red, area_red, &stat_red);
    }
    else
    {
        stat_red = stat_empty;
    }
    if (nele_yel != 0)
    {
        mystat(smax_yel, nele_yel, area_yel, &stat_yel);
    }
    else
    {
        stat_yel = stat_empty;
    }
    if (nele_wht != 0)
    {
        mystat(smax_wht, nele_wht, area_wht, &stat_wht);
    }
    else
    {
        stat_wht = stat_empty;
    }
    mystat(smax_aneu, nele_aneu, area_aneu, &stat_aneu);
    mystat(smax_dom, nele_dom, area_dom, &stat_dom);
    mystat(smax_bod, nele_bod, area_bod, &stat_bod);
    mystat(smax_nek, nele_nek, area_nek, &stat_nek);
    mystat(smax_part, nele_part, area_part, &stat_part);
    mystat(smax_press, nele_press, area_press, &stat_press);
    printf("red: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_red, nele_red), stat_red[0], stat_red[1], stat_red[2], stat_red[3]);
    printf("yel: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_yel, nele_yel), stat_yel[0], stat_yel[1], stat_yel[2], stat_yel[3]);
    printf("wht: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_wht, nele_wht), stat_wht[0], stat_wht[1], stat_wht[2], stat_wht[3]);
    printf("dom: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_dom, nele_dom), stat_dom[0], stat_dom[1], stat_dom[2], stat_dom[3]);
    printf("bod: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_bod, nele_bod), stat_bod[0], stat_bod[1], stat_bod[2], stat_bod[3]);
    printf("nek: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_nek, nele_nek), stat_nek[0], stat_nek[1], stat_nek[2], stat_nek[3]);
    printf("ane: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_aneu, nele_aneu), stat_aneu[0], stat_aneu[1], stat_aneu[2], stat_aneu[3]);
    printf("par: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_part, nele_part), stat_part[0], stat_part[1], stat_part[2], stat_part[3]);
    printf("pre: area: %0.5lf mean: %9.2lf max: %9.2lf min: %9.2lf stddev: %9.2lf\n", sumarr(area_press, nele_press), stat_press[0], stat_press[1], stat_press[2], stat_press[3]);

    // save in a table txt
    // arrays of different types and sizes
    char *arr1[4];
    for (int i = 0; i < 4; i++)
    {
        arr1[i] = (char *)malloc(strlen(casename) + 1); // +1 for the null terminator
        if (arr1[i] != NULL)
        {
            strcpy(arr1[i], casename);
        }
        else
        {
            // Handle memory allocation failure
            fprintf(stderr, "ERROR: Memory allocation failed for arr1[%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }
    char *arr2[4];
    for (int i = 0; i < 4; i++)
    {
        arr2[i] = (char *)malloc(strlen(study) + 1); // +1 for the null terminator
        if (arr2[i] != NULL)
        {
            strcpy(arr2[i], study);
        }
        else
        {
            // Handle memory allocation failure
            fprintf(stderr, "ERROR: Memory allocation failed for arr2[%d]\n", i);
            exit(EXIT_FAILURE);
        }
    }
    char *arr3[4] = {"mean", "max", "min", "stddev"};

    // Number of arrays and their sizes
    int numArrays = 12;
    int sizes[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}; // Sizes of the arrays

    // Array of pointers to the arrays
    void *arrays[] = {(void *)arr1, (void *)arr2, (void *)arr3, (void *)stat_aneu, (void *)stat_red, (void *)stat_yel, (void *)stat_wht, (void *)stat_dom,
                      (void *)stat_bod, (void *)stat_nek, (void *)stat_part, (void *)stat_press};

    // Data types of each array (int, float, char, string)
    DataType types[] = {STRING_TYPE, STRING_TYPE, STRING_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE, DOUBLE_TYPE};

    // Array of column headers (names)
    const char *headers[] = {"Casename", "Study", "stat_para", "stat_aneu", "stat_red", "stat_yel", "stat_wht", "stat_dom", "stat_bod", "stat_nek", "stat_part", "stat_press"};

    // Save the arrays to the file with headers
    saveMultipleArraysToFile("output.txt", numArrays, arrays, sizes, types, headers);

    // free memory
    for (int i = 0; i < 4; i++)
    {
        free(arr1[i]); // Free memory allocated for arr1
        free(arr2[i]); // Free memory allocated for arr2
    }

    // Free the dynamically allocated smax and area arrays
    free(smax_red);
    free(smax_yel);
    free(smax_wht);
    free(smax_aneu);
    free(smax_press);
    free(smax_dom);
    free(smax_bod);
    free(smax_nek);
    free(smax_part);

    free(area_red);
    free(area_yel);
    free(area_wht);
    free(area_aneu);
    free(area_press);
    free(area_dom);
    free(area_bod);
    free(area_nek);
    free(area_part);

    // Free dynamically allocated stat arrays if they are not pointing to stat_empty
    if (stat_red != stat_empty)
        free(stat_red);
    if (stat_yel != stat_empty)
        free(stat_yel);
    if (stat_wht != stat_empty)
        free(stat_wht);
    // Other stat arrays that were dynamically allocated
    free(stat_aneu);
    free(stat_dom);
    free(stat_bod);
    free(stat_nek);
    free(stat_part);
    free(stat_press);

    return e;
}
// Calling defined statictic analysis function in ordered
void mystat(double *arr, int n, double *area, double **output1)
{
    static double *output;
    output = calloc((size_t)5, sizeof(*output));
    output[0] = calculate_mean(arr, n, area);
    output[1] = find_max(arr, n);
    output[2] = find_min(arr, n);
    output[3] = calculate_stddev(arr, n, output[0], area);
    *output1 = output;
}
// find unidirectional or bidirectional stress region mask:
int unibimask(mesh *M, double *smax1, double *smax2, int **sdir2)
{
    int e = 0;
    double threshold = 229756; // (mean_press-stddev_press)  ~172mmHg
    int *sdir;
    sdir = calloc((size_t)M->nelem, sizeof(*sdir));
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->presmask[ele] == 0)
            continue;
        double hill = (1 / (1 + pow((fabs(smax2[ele]) / fabs(smax1[ele])), 2)));
        if (hill < 0.65 && hill > 0.5)
        {
            sdir[ele] = 4;
        }
        else
        {
            sdir[ele] = 3;
        }
        if (fabs(smax1[ele]) <= threshold)
        {
            if (sdir[ele] == 4)
                sdir[ele] = 2;
            if (sdir[ele] == 3)
                sdir[ele] = 1;
        }
    }
    *sdir2 = sdir;
    return e;
}
// sorted the eigenvalue and eignvector according to the normal of tri3 and max of eignvalues
// s_sorted [s_normal smax_shear, smax2_shear]
// vsorted  [v_vormal[] vmax_shear[], vmax2_shear[]]
int sortedsv(mesh *M, double *eigenvalue, double *eigenvector, double **sorted_s2, double **sorted_v2)
{
    int e = 0;
    double *sorted_s;
    double *sorted_v;
    sorted_s = calloc(3 * (size_t)M->nelem, sizeof(*sorted_s));
    sorted_v = calloc(9 * (size_t)M->nelem, sizeof(*sorted_v));

    double *s1, *s2, *s3, *v1, *v2, *v3;
    s1 = calloc((size_t)M->nelem, sizeof(*s1));
    s2 = calloc((size_t)M->nelem, sizeof(*s2));
    s3 = calloc((size_t)M->nelem, sizeof(*s3));
    v1 = calloc(3 * (size_t)M->nelem, sizeof(*v1));
    v2 = calloc(3 * (size_t)M->nelem, sizeof(*v2));
    v3 = calloc(3 * (size_t)M->nelem, sizeof(*v3));
    for (int ele = 0; ele < M->nelem; ele++)
    {
        s1[ele] = eigenvalue[3 * ele];
        s2[ele] = eigenvalue[3 * ele + 1];
        s3[ele] = eigenvalue[3 * ele + 2];
        for (int j = 0; j < 3; j++)
        {
            v1[3 * ele + j] = eigenvector[9 * ele + 3 * j + 0];
            v2[3 * ele + j] = eigenvector[9 * ele + 3 * j + 1];
            v3[3 * ele + j] = eigenvector[9 * ele + 3 * j + 2];
        }
    }

    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->presmask[ele] == 0)
            continue;
        double cos_teta1 = 0;
        double cos_teta2 = 0;
        double cos_teta3 = 0;
        for (int i = 0; i < 3; i++)
            cos_teta1 += M->normele[3 * ele + i] * v1[3 * ele + i];
        for (int i = 0; i < 3; i++)
            cos_teta2 += M->normele[3 * ele + i] * v2[3 * ele + i];
        for (int i = 0; i < 3; i++)
            cos_teta3 += M->normele[3 * ele + i] * v3[3 * ele + i];

        int normalV = 1;
        if (fabs(cos_teta1) < fabs(cos_teta2))
        {
            normalV = 2;
            cos_teta1 = cos_teta2;
        }
        if (fabs(cos_teta1) < fabs(cos_teta3))
        {
            normalV = 3;
        }
        if (normalV == 1)
        {
            sorted_s[3 * ele] = s1[ele];
            for (int i = 0; i < 3; i++)
                sorted_v[9 * ele + i] = v1[3 * ele + i];
            int maxshear = 2;
            if (fabs(s2[ele]) < fabs(s3[ele]))
                maxshear = 3;
            if (maxshear == 2)
            {
                sorted_s[3 * ele + 1] = s2[ele];
                sorted_s[3 * ele + 2] = s3[ele];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 3 + i] = v2[3 * ele + i];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 6 + i] = v3[3 * ele + i];
            }
            if (maxshear == 3)
            {
                sorted_s[3 * ele + 1] = s3[ele];
                sorted_s[3 * ele + 2] = s2[ele];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 3 + i] = v3[3 * ele + i];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 6 + i] = v2[3 * ele + i];
            }
        }
        if (normalV == 2)
        {
            sorted_s[3 * ele] = s2[ele];
            for (int i = 0; i < 3; i++)
                sorted_v[9 * ele + i] = v2[3 * ele + i];
            int maxshear = 1;
            if (fabs(s1[ele]) < fabs(s3[ele]))
                maxshear = 3;
            if (maxshear == 1)
            {
                sorted_s[3 * ele + 1] = s1[ele];
                sorted_s[3 * ele + 2] = s3[ele];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 3 + i] = v1[3 * ele + i];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 6 + i] = v3[3 * ele + i];
            }
            if (maxshear == 3)
            {
                sorted_s[3 * ele + 1] = s3[ele];
                sorted_s[3 * ele + 2] = s1[ele];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 3 + i] = v3[3 * ele + i];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 6 + i] = v1[3 * ele + i];
            }
        }
        if (normalV == 3)
        {
            sorted_s[3 * ele] = s3[ele];
            for (int i = 0; i < 3; i++)
                sorted_v[9 * ele + i] = v3[3 * ele + i];
            int maxshear = 1;
            if (fabs(s1[ele]) < fabs(s2[ele]))
                maxshear = 2;
            if (maxshear == 1)
            {
                sorted_s[3 * ele + 1] = s1[ele];
                sorted_s[3 * ele + 2] = s2[ele];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 3 + i] = v1[3 * ele + i];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 6 + i] = v2[3 * ele + i];
            }
            if (maxshear == 2)
            {
                sorted_s[3 * ele + 1] = s2[ele];
                sorted_s[3 * ele + 2] = s1[ele];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 3 + i] = v2[3 * ele + i];
                for (int i = 0; i < 3; i++)
                    sorted_v[9 * ele + 6 + i] = v1[3 * ele + i];
            }
        }
    }
    free(s1);
    free(s2);
    free(s3);
    free(v1);
    free(v2);
    free(v3);
    *sorted_s2 = sorted_s;
    *sorted_v2 = sorted_v;
    printf("* sorted eigen vector and eigen values according to the normal vector.\n");
    return e;
}
// read stress tensor in the log file of febio
int readfebiolog(char *path, mesh *M, double **st2, read_time logtime)
{

    int e = 0;
    double *st;
    // Open log file :
    FILE *fptr = fopen(path, "r");
    char str[256];
    if (fptr == NULL)
    {
        fprintf(stderr, "* ERROR: there is no file in this path : %s.\n", path);
        exit(EXIT_FAILURE);
    }
    // find the number of shell element
    int nelem = 0;
    while (fgets(str, 256, fptr) != NULL)
    {
        if (sscanf(str, "	Number of shell elements ....................... : %d", &nelem) == 1)
            break;
    }
    fclose(fptr);
    printf("* number of shell element is %d\n", nelem);
    if (M->nelem != nelem)
    {
        fprintf(stderr, "ERROR: Number of shell element in the file : %s does not match with what zfem file on data directory", path);
        exit(EXIT_FAILURE);
    }
    // find the maximum time in the file
    double time_value, max_time;
    max_time = 0.0;
    fptr = fopen(path, "r");
    while (fgets(str, 256, fptr) != NULL)
    {
        if (sscanf(str, "Time = %lf", &time_value) == 1)
            max_time = MAX(time_value, max_time);
    }
    fclose(fptr);
    printf("* %s study execute till time = %lf\n", path, max_time);
    // save the stress tensor
    st = calloc(9 * (size_t)nelem, sizeof(*st));
    double logtime_value = 0.0;
    if (logtime == end_first_step)
        logtime_value = 1.0;
    if (logtime == end_second_step)
        logtime_value = 2.0;
    if (logtime == time_max)
        logtime_value = max_time;
    // st [9] = [sxx,sxy,sxz;syx,syy,syz;szx,szy,szz]
    fptr = fopen(path, "r");
    int junk, nscan = 0;
    while (fgets(str, 256, fptr) != NULL)
    {
        if (sscanf(str, "Time = %lf", &time_value) == 1)
        {
            if (time_value == logtime_value)
            {
                fgets(str, 256, fptr);
                for (int ele = 0; ele < nelem; ele++)
                {
                    fgets(str, 256, fptr);
                    nscan = 0;
                    // log_st[6]= [sxx(0),syy(4),szz(8),sxy(1)(3),syz(5)(7),sxz(2)(6)]
                    nscan = sscanf(str, "%d %lf %lf %lf %lf %lf %lf", &junk, &st[9 * ele], &st[9 * ele + 4], &st[9 * ele + 8],
                                   &st[9 * ele + 1], &st[9 * ele + 5], &st[9 * ele + 2]);
                    if (nscan != 7)
                    {
                        fprintf(stderr, "there is error on number of element in line %d", ele);
                        exit(EXIT_FAILURE);
                    }
                    st[9 * ele + 3] = st[9 * ele + 1];
                    st[9 * ele + 7] = st[9 * ele + 5];
                    st[9 * ele + 6] = st[9 * ele + 2];
                }
            }
        }
    }
    // for(int ele=0;ele<10;ele++){
    //     printf("ele: %d ",ele);
    //     for(int i =0;i<9;i++) printf("%lf ",st[9*ele+i]);
    //     printf("\n");
    // }
    fclose(fptr);
    printf("* the stress tensor saved !\n");

    *st2 = st;
    return e;
}