#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mystructs.h"
#include "common.h"
#include "myfuncs.h"
#include "febiofuncs.h"

int read_zfem(char *path, int *npoin, int *nelem, double **ptxyz, int **elems)
{
    int e = 0;
    int npoin1 = 0, nelem1 = 0;
    int *elems1 = NULL;
    double *ptxyz1 = NULL;

    /* Open the file */
    FILE *fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        return -1;
    }
    printf("File opened - %s.\n", path);

    /* Read all lines of the file */
    int buffer = 100;
    char *str;
    char line[buffer];
    int endcount = 0, nscan, iline;
    char test[20];

    while (1)
    {
        // Start reading points
        str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%s", test);

        if (!strcmp(test, "POINTS"))
        {
            printf("Reading POINTS.\n");

            /* Read Number of Points */
            str = edit_endline_character(line, buffer, fptr);
            nscan = sscanf(str, "%d", &npoin1);
            printf("Number of Points = %d.\n", npoin1);
            if (nscan != 1)
            {
                fprintf(stderr, "ERROR: Incorrect number of entries on POINTS line.\n");
                fclose(fptr);
                return -1;
            }

            /* Allocate and Read Coordinates */
            ptxyz1 = malloc(dimension * (size_t)npoin1 * sizeof(*ptxyz1));
            if (!ptxyz1)
            {
                fprintf(stderr, "ERROR: Memory allocation failed for ptxyz array.\n");
                fclose(fptr);
                return -1;
            }
            for (iline = 0; iline < npoin1; iline++)
            {
                str = edit_endline_character(line, buffer, fptr);
                nscan = sscanf(str, "%lf %lf %lf",
                               &ptxyz1[dimension * iline],
                               &ptxyz1[dimension * iline + 1],
                               &ptxyz1[dimension * iline + 2]);
                if (nscan != 3)
                {
                    fprintf(stderr, "ERROR: Incorrect coordinates on line %d of POINTS.\n", iline + 1);
                    free(ptxyz1);
                    fclose(fptr);
                    return -1;
                }
            }
            endcount += 1;
        }
        else if (!strcmp(test, "TRIANGLE"))
        {
            printf("Reading ELEMENTS.\n");

            /* Read Number of Elements */
            str = edit_endline_character(line, buffer, fptr);
            str = edit_endline_character(line, buffer, fptr);
            nscan = sscanf(str, "%d", &nelem1);
            printf("Number of ELEMENTS = %d.\n", nelem1);
            if (nscan != 1)
            {
                fprintf(stderr, "ERROR: Incorrect number of entries for ELEMENTS.\n");
                free(ptxyz1);
                fclose(fptr);
                return -1;
            }

            /* Allocate and Read Connectivity */
            elems1 = malloc(3 * (size_t)nelem1 * sizeof(*elems1));
            if (!elems1)
            {
                fprintf(stderr, "ERROR: Memory allocation failed for elems array.\n");
                free(ptxyz1);
                fclose(fptr);
                return -1;
            }

            for (iline = 0; iline < nelem1; iline++)
            {
                str = edit_endline_character(line, buffer, fptr);
                nscan = sscanf(str, "%d %d %d",
                               &elems1[3 * iline],
                               &elems1[3 * iline + 1],
                               &elems1[3 * iline + 2]);
                if (nscan != 3)
                {
                    fprintf(stderr, "ERROR: Incorrect connectivity on line %d of ELEMENTS.\n", iline + 1);
                    free(ptxyz1);
                    free(elems1);
                    fclose(fptr);
                    return -1;
                }
            }
            endcount += 1;
        }

        // Break loop if both POINTS and ELEMENTS sections are read
        if (endcount == 2)
            break;
    }

    /* Close the file */
    if (fclose(fptr) == EOF)
    {
        fprintf(stderr, "ERROR: Failed to close file %s.\n", path);
        free(ptxyz1);
        free(elems1);
        return -1;
    }

    // Assign results to output pointers
    *npoin = npoin1;
    *nelem = nelem1;
    *ptxyz = ptxyz1;
    *elems = elems1;

    printf("* Exiting function for reading flds.zfem file.\n");
    checkEIDS(elems1); // Call any post-processing/check function as needed

    return e;
}
int read_BCmask(char *path, mesh *M, int **BCmask2)
{
    int *arr = calloc((size_t)M->nelem, sizeof(*arr));
    if (arr == NULL)
    {
        fprintf(stderr, "Memory allocation failed for BCmask array.\n");
        return -1;
    }

    // Open the file
    FILE *fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        free(arr); // Free allocated memory before returning
        return -1;
    }
    printf("  File opened - %s.\n", path);

    /* Read all lines of the file */
    int buffer = 100;
    char *str, line[buffer];
    int nscan, iline;

    for (iline = 0; iline < M->nelem; iline++)
    {
        str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%d", &arr[iline]);

        if (nscan != 1)
        {
            fprintf(stderr, "ERROR: Error reading line %d in file %s.\n", iline + 1, path);
            free(arr);    // Free allocated memory
            fclose(fptr); // Close file before returning
            return -1;
        }
    }

    // Close the file
    if (fclose(fptr) == EOF)
    {
        fprintf(stderr, "ERROR: Failed to close file %s.\n", path);
        free(arr); // Free allocated memory before returning
        return -1;
    }

    // Assign the allocated array to the output pointer
    *BCmask2 = arr;
    return 0;
}
int read_wallmask(char *path, mesh *M, input *inp, int **Melem2)
{
    int e = 0;
    int *Melem = calloc((size_t)M->nelem, sizeof(*Melem));
    if (Melem == NULL)
    {
        fprintf(stderr, "Memory allocation failed for Melem.\n");
        return -1;
    }

    /* Open the file */
    FILE *fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        free(Melem); // Free allocated memory before returning
        return -1;
    }
    printf("  File opened - %s.\n", path);

    /* Read all lines of the file */
    int buffer = 100;
    char *str, line[buffer];
    int nscan, iline;

    /* Read labels of all elements */
    int temp = 0;
    for (iline = 0; iline < M->nelem; iline++)
    {
        str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%d", &temp);
        if (nscan != 1)
        {
            fprintf(stderr, "ERROR: Incorrect number of entries on POINTS line.\n");
            free(Melem);  // Free before returning
            fclose(fptr); // Close the file before returning
            return -1;
        }

        // Check the value of the Melem
        for (int k = 0; k < inp->label_num; k++)
        {
            if (temp == inp->label[k])
            {
                Melem[iline] = inp->label[k];
            }
        }
    }

    /* Close the file */
    if (fclose(fptr) == EOF)
    {
        printf("Error closing %s\n", path);
        free(Melem); // Free before returning in case of error
        return -1;
    }

    /* Assign the allocated array to the output pointer */
    *Melem2 = Melem;
    printf("*  Exiting function for reading .WALL mask file!\n");

    return e;
}
int read_regionmask(char *path, mesh *M, input *inp, int **region_id2, int **region_idp2)
{
    int e = 0;

    // Allocate memory for region_id
    int *region_id = malloc((size_t)M->npoin * sizeof(*region_id));
    if (region_id == NULL)
    {
        fprintf(stderr, "Memory allocation failed for region_id.\n");
        return -1;
    }

    // Open the file
    FILE *fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        free(region_id); // Free allocated memory before returning
        return -1;
    }

    /* Read all lines of the file */
    int buffer = 100;
    char *str;
    char line[buffer];
    int endcount = 0;
    int nscan, iline;
    char test[20];

    // Reading region labels
    while (1)
    {
        str = edit_endline_character(line, buffer, fptr);
        nscan = sscanf(str, "%s", test);

        if (!strcmp(test, "regions"))
        {
            str = edit_endline_character(line, buffer, fptr);
            nscan = sscanf(str, "%s", test);

            if (!strcmp(test, "1"))
            {
                for (iline = 0; iline < M->npoin; iline++)
                {
                    str = edit_endline_character(line, buffer, fptr);
                    nscan = sscanf(str, "%d", &region_id[iline]);

                    // Error check
                    if (nscan != 1)
                    {
                        fprintf(stderr, "ERROR: Incorrect data on line %d.\n", iline + 1);
                        free(region_id);
                        fclose(fptr);
                        return -1;
                    }

                    // Check if value is in colorid array
                    int checkflag = 0;
                    for (int k = 0; k < inp->colorid_num; k++)
                    {
                        if (region_id[iline] == inp->colorid[k])
                        {
                            checkflag++;
                            break;
                        }
                    }
                    if (checkflag != 1)
                    {
                        printf("ERROR: Value of element %d (value=%d) is not in colorId array\n", iline, region_id[iline]);
                        free(region_id);
                        fclose(fptr);
                        return -1;
                    }
                }
                endcount += 1;
            }
        }

        if (endcount == 1)
        {
            printf("* Done Reading region mask file!\n");
            break;
        }
    }

    // Close the file
    if (fclose(fptr) == EOF)
    {
        printf("Error closing %s\n", path);
        free(region_id);
        return -1;
    }

    // Allocate memory for region_id_ele
    int *region_id_ele = malloc((size_t)M->nelem * sizeof(*region_id_ele));
    if (region_id_ele == NULL)
    {
        fprintf(stderr, "Memory allocation failed for region_id_ele.\n");
        free(region_id);
        return -1;
    }

    // Fill region_id_ele based on region_id
    int points[3] = {0, 0, 0};
    for (int ele = 0; ele < M->nelem; ele++)
    {
        points[0] = M->elems[3 * ele];
        points[1] = M->elems[3 * ele + 1];
        points[2] = M->elems[3 * ele + 2];

        // Assign region ID for the element based on one point (e.g., points[0] - 1)
        region_id_ele[ele] = region_id[points[0] - 1];
    }

    /* Return results */
    *region_id2 = region_id_ele;
    *region_idp2 = region_id;

    return e;
}
