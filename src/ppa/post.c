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
    // read a
    static void *field1;
    FunctionWithArgs2 prtreadfield[] = {
        {"Press_mask", 1, M1->nelem, &field1, read_VTK_int},
    };
    int countfield = sizeof(prtreadfield) / sizeof(prtreadfield[0]);
    CHECK_ERROR(ReadVTK(pst_rundir, "checkinput", 0, prtreadfield, countfield));
    M1->presmask = (int *)field1;
    // write result in VTK format
    FunctionWithArgs prtelefield[] =
        {
            {"S1", 1, M1->nelem, s1, SCA_double_VTK},
            {"S2", 1, M1->nelem, s2, SCA_double_VTK},
            {"S3", 1, M1->nelem, s3, SCA_double_VTK},            
            {"V1", 3, M1->nelem, v1, VEC_double_VTK},
            {"V2", 3, M1->nelem, v2, VEC_double_VTK},
            {"V3", 3, M1->nelem, v3, VEC_double_VTK},
        };
    size_t countele = sizeof(prtelefield) / sizeof(prtelefield[0]);
    FunctionWithArgs prtpntfield[] = {NULL};
    size_t countpnt = 0;
    CHECK_ERROR(SaveVTK(pstdir, "stress", atoi(iteration), M1, tri3funcVTK, prtelefield, countele, prtpntfield, countpnt));

    // find the max principale stress for all domain;
    int ns1 = 0;
    int ns2 = 0;
    int ns3 = 0;
    for (int ele = 0; ele < M1->nelem; ele++)
    {
        if (M1->presmask[ele] == 0)
            continue;
        double max = s1[ele];
        int which = 1;
        if (max > s2[ele])
        {
            max = s2[ele];
            which = 2;
        }
        if (max > s3[ele])
        {
            which = 3;
        }
        if (which == 1)
            ns1++;
        if (which == 2)
            ns2++;
        if (which == 3)
            ns3++;
    }
    printf("* number of nelem: %d ns1: %d ns2: %d ns3: %d\n", M1->nelem, ns1, ns2, ns3);
    // // Example arrays of different types and sizes
    // int arr1[] = {1, 2, 3, 4};                          // Array of integers
    // float arr2[] = {10.5, 20.75, 30.125};               // Array of floats
    // char arr3[] = {'A', 'B', 'C', 'D'};                 // Array of chars
    // char *arr4[] = {"Hello", "World", "C", "Language"}; // Array of strings

    // // Number of arrays and their sizes
    // int numArrays = 4;
    // int sizes[] = {4, 3, 4, 4}; // Sizes of the arrays

    // // Array of pointers to the arrays
    // void *arrays[] = {(void *)arr1, (void *)arr2, (void *)arr3, (void *)arr4};

    // // Data types of each array (int, float, char, string)
    // DataType types[] = {INT_TYPE, FLOAT_TYPE, CHAR_TYPE, STRING_TYPE};

    // // Array of column headers (names)
    // const char *headers[] = {"Integers", "Floats", "Characters", "Strings"};

    // // Save the arrays to the file with headers
    // saveMultipleArraysToFile("output.txt", numArrays, arrays, sizes, types, headers);

    free(st);
    free(s1);
    free(s2);
    free(s3);
    free(v1);
    free(v2);
    free(v3);
    free(eigenvalue);
    free(eigenvector);
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
