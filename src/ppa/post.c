#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mystructs.h"
#include "common.h"
#include "myfuncs.h"
#include "febiofuncs.h"

// declare global parameters
// directories:
char pst_rundir[50];
char pstdir[50] = "./";
char pst_datadir[50];
// data file
char past_datafilepath[10][500];
// past_filename
char febname[10] = "pres";
char past_filename[50];
char study[50];
char iteration[50];

int files(void);
int dirs(void);
void mystat(double *arr, int n, double *area, double **output1);

int main(int argc, char const **argv)
{
    // check the argument
    if (argc != 4)
    {
        fprintf(stderr, "argc is %d\n", argc);
        fprintf(stderr, "ERROR: need more argument\n");
        fprintf(stderr, "<casename> <study> <iteration> \n\n");
        fprintf(stderr, "a06161.1 msa.1 0\n\n");
        exit(EXIT_FAILURE);
    }
    strcpy(past_filename, argv[1]);
    strcpy(study, argv[2]);
    strcpy(iteration, argv[3]);
    CHECK_ERROR(dirs());
    CHECK_ERROR(files());
    printf("- %s\n", past_datafilepath[0]);
    printf("- %s\n", past_datafilepath[1]);

    // find the maximum time in the file
    FILE *fptr = fopen(past_datafilepath[1], "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "* ERROR: there is no file in this path : %s.\n", past_datafilepath[1]);
        exit(EXIT_FAILURE);
    }
    char str[256];
    int time_value, max_time = 0;
    while (fgets(str, 256, fptr) != NULL)
    {
        if (sscanf(str, "Time = %d", &time_value) == 1)
            max_time = MAX(time_value, max_time);
    }
    fclose(fptr);
    // find the number of shell element
    printf("* this study execute till time = %d\n", max_time);
    int nelem = 0;
    fptr = fopen(past_datafilepath[1], "r");
    while (fgets(str, 256, fptr) != NULL)
    {
        if (sscanf(str, "	Number of shell elements ....................... : %d", &nelem) == 1)
            break;
    }
    fclose(fptr);
    printf("* number of shell element is %d\n", nelem);
    // save the stress tensor
    double *st;
    st = calloc(9 * (size_t)nelem, sizeof(*st));
    // st [9] = [sxx,sxy,sxz;syx,syy,syz;szx,szy,szz]
    fptr = fopen(past_datafilepath[1], "r");
    int junk, nscan = 0;
    while (fgets(str, 256, fptr) != NULL)
    {
        if (sscanf(str, "Time = %d", &time_value) == 1)
        {
            if (time_value == max_time)
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

    // find the eigenvectors and eigenvalues of stress tensor
    double *eigenvalue, *eigenvector;
    CHECK_ERROR(jacobiMethod(nelem, st, &eigenvalue, &eigenvector));
    double *s1, *s2, *s3, *v1, *v2, *v3;
    s1 = calloc((size_t)nelem, sizeof(*s1));
    s2 = calloc((size_t)nelem, sizeof(*s2));
    s3 = calloc((size_t)nelem, sizeof(*s3));
    v1 = calloc(3 * (size_t)nelem, sizeof(*v1));
    v2 = calloc(3 * (size_t)nelem, sizeof(*v2));
    v3 = calloc(3 * (size_t)nelem, sizeof(*v3));
    for (int ele = 0; ele < nelem; ele++)
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
    // reading domain parameters for unloaded geometry from .FLDS.ZFEM file //
    mesh *M1 = (mesh *)malloc(sizeof(mesh));
    CHECK_ERROR(read_zfem(past_datafilepath[0], &M1->npoin, &M1->nelem, &M1->ptxyz, &M1->elems));
    if (M1->nelem != nelem)
    {
        fprintf(stderr, "ERROR: number of shell element in log file not math with zfem file in data directory!\n");
        exit(EXIT_FAILURE);
    }
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
    // sorted eigen vectors and eigen values:
    double *sorted_s;
    double *sorted_v;
    sorted_s = calloc(3 * (size_t)M1->nelem, sizeof(*sorted_s));
    sorted_v = calloc(9 * (size_t)M1->nelem, sizeof(*sorted_v));
    for (int ele = 0; ele < M1->nelem; ele++)
    {
        if (M1->presmask[ele] == 0)
            continue;
        double cos_teta1 = 0;
        double cos_teta2 = 0;
        double cos_teta3 = 0;
        for (int i = 0; i < 3; i++)
            cos_teta1 += M1->normele[3 * ele + i] * v1[3 * ele + i];
        for (int i = 0; i < 3; i++)
            cos_teta2 += M1->normele[3 * ele + i] * v2[3 * ele + i];
        for (int i = 0; i < 3; i++)
            cos_teta3 += M1->normele[3 * ele + i] * v3[3 * ele + i];

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
    double *smax1, *smax2, *vmax1, *vmax2;
    smax1 = calloc((size_t)M1->nelem, sizeof(*smax1));
    smax2 = calloc((size_t)M1->nelem, sizeof(*smax2));
    vmax1 = calloc(3 * (size_t)M1->nelem, sizeof(*vmax1));
    vmax2 = calloc(3 * (size_t)M1->nelem, sizeof(*vmax2));
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

    // find unidirectional or bidirectional stress region mask:
    double threshold = 229756; // (mean_press-stddev_press)  ~172mmHg
    int *sdir;
    sdir = calloc((size_t)M1->nelem, sizeof(*sdir));
    for (int ele = 0; ele < M1->nelem; ele++)
    {
        if (M1->presmask[ele] == 0)
            continue;
        double hill = (1 / (1 + pow((fabs(smax2[ele]) / fabs(smax1[ele])), 4)));
        if (hill < 0.75 && hill > 0.25)
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

    // write result in VTK format
    FunctionWithArgs prtelefield[] =
        {
            {"S1", 1, M1->nelem, s1, SCA_double_VTK},
            {"S2", 1, M1->nelem, s2, SCA_double_VTK},
            {"S3", 1, M1->nelem, s3, SCA_double_VTK},
            {"smax1", 1, M1->nelem, smax1, SCA_double_VTK},
            {"smax2", 1, M1->nelem, smax2, SCA_double_VTK},
            {"sdir", 1, M1->nelem, sdir, SCA_int_VTK},
            {"V1", 3, M1->nelem, v1, VEC_double_VTK},
            {"V2", 3, M1->nelem, v2, VEC_double_VTK},
            {"V3", 3, M1->nelem, v3, VEC_double_VTK},
            {"vmax1", 3, M1->nelem, vmax1, VEC_double_VTK},
            {"vmax2", 3, M1->nelem, vmax2, VEC_double_VTK},
            {"normele", 3, M1->nelem, M1->normele, VEC_double_VTK},
        };
    size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
    FunctionWithArgs prtpntfield[] = {NULL};
    size_t countpnt = 0;
    CHECK_ERROR(SaveVTK(pstdir, "stress", atoi(iteration), M1, tri3funcVTK, prtelefield, countele, prtpntfield, countpnt));

    // analysis the max stress @ color region mask and region mask
    int nele_red, nele_yel, nele_wht, nele_press, nele_dom, nele_bod, nele_nek, nele_part, nele_aneu;
    nele_red = nele_yel = nele_wht = nele_press = nele_dom = nele_bod = nele_nek = nele_part = nele_aneu = 0;
    double *smax_red, *smax_yel, *smax_wht, *smax_press, *smax_aneu, *smax_dom, *smax_bod, *smax_nek, *smax_part;
    double *area_red, *area_yel, *area_wht, *area_press, *area_aneu, *area_dom, *area_bod, *area_nek, *area_part;
    // find the size of each domain
    for (int ele = 0; ele < M1->nelem; ele++)
    {
        if (M1->presmask[ele] == 0)
            continue;
        nele_press++;
        if (M1->Melem[ele] == 1)
            nele_red++;
        if (M1->Melem[ele] == 2)
            nele_yel++;
        if (M1->Melem[ele] == 7)
            nele_wht++;
        if (M1->relems[ele] == 16)
            nele_dom++;
        if (M1->relems[ele] == 8)
            nele_bod++;
        if (M1->relems[ele] == 4)
            nele_nek++;
        if (M1->relems[ele] == 1)
            nele_part++;
        if (M1->relems[ele] == 4 || M1->relems[ele] == 8 || M1->relems[ele] == 16)
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
    for (int ele = 0; ele < M1->nelem; ele++)
    {
        if (M1->presmask[ele] == 0)
            continue;
        smax_press[nele_press] = fabs(smax1[ele]);
        area_press[nele_press] = area[ele];
        nele_press++;
        if (M1->Melem[ele] == 1)
        {
            smax_red[nele_red] = fabs(smax1[ele]);
            area_red[nele_red] = area[ele];
            nele_red++;
        }

        if (M1->Melem[ele] == 2)
        {
            smax_yel[nele_yel] = fabs(smax1[ele]);
            area_yel[nele_yel] = area[ele];
            nele_yel++;
        }

        if (M1->Melem[ele] == 7)
        {
            smax_wht[nele_wht] = fabs(smax1[ele]);
            area_wht[nele_wht] = area[ele];
            nele_wht++;
        }

        if (M1->relems[ele] == 16)
        {
            smax_dom[nele_dom] = fabs(smax1[ele]);
            area_dom[nele_dom] = area[ele];
            nele_dom++;
        }

        if (M1->relems[ele] == 8)
        {
            smax_bod[nele_bod] = fabs(smax1[ele]);
            area_bod[nele_bod] = area[ele];
            nele_bod++;
        }

        if (M1->relems[ele] == 4)
        {
            smax_nek[nele_nek] = fabs(smax1[ele]);
            area_nek[nele_nek] = area[ele];
            nele_nek++;
        }

        if (M1->relems[ele] == 1)
        {
            smax_part[nele_part] = fabs(smax1[ele]);
            area_part[nele_part] = area[ele];
            nele_part++;
        }

        if (M1->relems[ele] == 4 || M1->relems[ele] == 8 || M1->relems[ele] == 16)
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
        arr1[i] = (char *)malloc(strlen(past_filename) + 1); // +1 for the null terminator
        if (arr1[i] != NULL)
        {
            strcpy(arr1[i], past_filename);
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

    free(st);
    free(s1);
    free(s2);
    free(s3);
    free(v1);
    free(v2);
    free(v3);
    free(smax1);
    free(smax2);
    free(eigenvalue);
    free(eigenvector);
    return 0;
}
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
    return e;
}
int dirs(void)
{
    strcpy(pst_rundir, "../");
    strcat(pst_rundir, study);
    strcat(pst_rundir, "/");
    strcpy(pst_datadir, "../data/");

    return 0;
}
