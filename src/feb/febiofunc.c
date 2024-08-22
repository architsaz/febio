#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mystructs.h"
#include "myfuncs.h"
#include "common.h"
#include "febiofuncs.h"

int runfebio(int step)
{
    int e = 0;
    //-------------> OPTION 1
    // /* define path */
    //     char path [500];
    //     strcpy(path,rundir);
    //     strcat(path,"runfebio.sh");

    // /* define File pointer:*/
    //     FILE *fptr;
    //     fptr = calloc(1, sizeof(*fptr));
    // /* Opening File */
    //     fptr = fopen(path, "w");
    //     if (fptr == NULL) {
    //     fprintf(stderr,"ERROR: Cannot open file - %s.\n", path);
    //     return -1;
    //     }
    // /*write & run runfeb file : */
    //     fprintf(fptr,"#!/bin/bash\n\n");
    //     fprintf(fptr,"febio4 -i %spres_%d.feb -config febio.xml\n",rundir,step);
    //     if (fclose(fptr) == EOF) {
    //         // If fclose returns EOF, it means there was an error closing the file
    //         printf("Error closing %s\n",path);
    //         return -1;
    //     }
    //     char command [500];
    //     sprintf(command,"./%srunfebio.sh",rundir);
    //     printf("%s\n",command);
    //     system(command);

    //---------------->OPTION 2
    char command[500];
    sprintf(command, "/dagon1/achitsaz/app/FEBioStudio/bin/febio4 -i %spres_%d.feb\n", rundir, step);
    printf("%s\n", command);
    system(command);
    return e;
}
int readNJ(int step)
{
    int e = 0;
    // ------------> OPTION 1
    // /* define path */
    // char path[500];
    // strcpy(path, rundir);
    // strcat(path, "runfebio.sh");

    // /* define File pointer:*/
    // FILE *fptr;
    // fptr = calloc(1, sizeof(*fptr));
    // /* Opening File */
    // fptr = fopen(path, "w");
    // if (fptr == NULL)
    // {
    //     fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
    //     return -1;
    // }
    // /*write & run runfeb file : */
    // fprintf(fptr, "#!/bin/bash\n\n");
    // fprintf(fptr, "grep \"Negative jacobian was detected at element\" %spres_%d.log | awk '{print $8}' >%sNJ.txt", rundir, step, rundir);
    // if (fclose(fptr) == EOF)
    // {
    //     // If fclose returns EOF, it means there was an error closing the file
    //     printf("Error closing %s\n", path);
    //     return -1;
    // }
    // char command[500];
    // sprintf(command, "./%srunfebio.sh", rundir);
    // printf("%s\n", command);
    // system(command);

    // --------------> OPTION 2
    char command[500];
    sprintf(command, "grep \"Negative jacobian was detected at element\" %spres_%d.log | awk '{print $8}' > %sNJ.txt", rundir, step, rundir);
    printf("%s\n", command);
    system(command);
    /*read the Nr of the Negative Jacobian elements*/
    /* define File pointer:*/
    FILE *fptr;
    fptr = calloc(1, sizeof(*fptr));
    char path[500];
    strcpy(path, rundir);
    strcat(path, "NJ.txt");
    NrNj = countline(path);
    /*read NJ.txt file and make NJ mask*/
    fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        return -1;
    }
    int buffer = 100;
    char *str, line[buffer];
    int nscan, *NJele;
    NJele = calloc((size_t)NrNj, sizeof(*NJele));
    for (int i = 0; i < NrNj; i++)
    {
        str = edit_endline_character(line, buffer, fptr);
        // printf("%s\n",str);
        nscan = sscanf(str, "%d", &NJele[i]);
        if (nscan != 1)
        {
            fprintf(stderr, "ERROR: Incorrect number of entries on line %d of NJ.txt.\n", i);
            return -1;
        }
    }
    for (int i = 0; i < NrNj; i++)
        NJmask[NJele[i]] = 1;
    return e;
}
int appliedgfilt_ptri6(mesh *M0, double *arrp, int clc)
{
    int e = 0;
    double *arre;
    arre = calloc((size_t)M0->nelem, sizeof(*arre));
    for (int iter = 0; iter < clc; iter++)
    {
        for (int ele = 0; ele < M0->nelem; ele++)
        {
            double mean = 0;
            for (int p = 0; p < M0->nrpts; p++)
                mean += arrp[(M0->elems[M0->nrpts * ele + p]) - 1];
            arre[ele] = mean / M0->nrpts;
        }
        for (int i = 1; i <= M0->npoin; i++)
        {
            double mean = 0;
            for (int j = M0->esurp_ptr[i]; j < M0->esurp_ptr[i + 1]; j++)
                mean += arre[M0->esurp[j]];
            arrp[i - 1] = mean / (M0->esurp_ptr[i + 1] - M0->esurp_ptr[i]);
        }
        for (int i = 0; i < M0->numf; i++)
        {
            arrp[M0->npoin + i] = (arrp[M0->psurf[2 * i] - 1] + arrp[M0->psurf[2 * i + 1] - 1]) / 2;
        }
    }
    free(arre);
    return e;
}
int appliedgfilt_etri(mesh *M0, double *arre, int clc)
{
    int e = 0;
    double *arrp;
    arrp = calloc((size_t)M0->npoin, sizeof(*arrp));
    for (int iter = 0; iter < clc; iter++)
    {
        for (int i = 1; i <= M0->npoin; i++)
        {
            double mean = 0;
            for (int j = M0->esurp_ptr[i]; j < M0->esurp_ptr[i + 1]; j++)
                mean += arre[M0->esurp[j]];
            arrp[i - 1] = mean / (M0->esurp_ptr[i + 1] - M0->esurp_ptr[i]);
        }
        for (int ele = 0; ele < M0->nelem; ele++)
        {
            double mean = 0;
            for (int p = 0; p < M0->nrpts; p++)
                mean += arrp[(M0->elems[M0->nrpts * ele + p]) - 1];
            arre[ele] = mean / M0->nrpts;
        }
    }
    free(arrp);
    return e;
}
runstatus checkresult(char *file)
{
    runstatus status = unknown;
    /* define path */
    // char path[500];
    // strcpy(path, rundir);
    // strcat(path, "runfebio.sh");

    // /* define File pointer:*/
    // FILE *fptr;
    // fptr = calloc(1, sizeof(*fptr));
    // /* Opening File */
    // fptr = fopen(path, "w");
    // if (fptr == NULL)
    // {
    //     fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
    //     exit(EXIT_FAILURE);
    // }
    // /*remove txt files*/
    // char command1[500] = "rm -r ";
    // strcat(command1, rundir);
    // strcat(command1, "*.txt");
    // system(command1);
    // /*write & run runfeb file : */
    // fprintf(fptr, "#!/bin/bash\n\n");
    // fprintf(fptr, "grep \"E R R O R\" %s%s.log > %sresult.txt", rundir, filename, rundir);
    // if (fclose(fptr) == EOF)
    // {
    //     // If fclose returns EOF, it means there was an error closing the file
    //     printf("Error closing %s\n", path);
    //     exit(EXIT_FAILURE);
    // }
    // char command[500];
    // sprintf(command, "./%srunfebio.sh", rundir);
    // printf("%s\n", command);
    // system(command);

    //------------------> OPTION2
    char command[500];
    sprintf(command, "grep \"E R R O R\" %s%s.log > %sresult.txt", rundir, file, rundir);
    printf("%s\n", command);
    system(command);
    sprintf(command, "grep \"N O R M A L\" %s%s.log >> %sresult.txt", rundir, file, rundir);
    printf("%s\n", command);
    system(command);

    char path[500];
    FILE *fptr;
    fptr = calloc(1, sizeof(*fptr));
    strcpy(path, rundir);
    strcat(path, "result.txt");
    /* Opening File */
    fptr = fopen(path, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        exit(EXIT_FAILURE);
    }
    int c;
    do
    {
        c = fgetc(fptr);
        if (c == 'E')
        {
            status = error;
            break;
        }
        if (c == 'N')
        {
            status = normal;
            break;
        }

        if (feof(fptr))
            break;
    } while (1);
    fclose(fptr);
    return status;
}
int calctrithick(mesh *M, input *inp)
{
    int e = 0;
    static double *t;
    t = malloc((size_t)M->npoin * sizeof(*t));
    // applied region mask
    if (inp->used_rmask==1){
        for (int ele = 0; ele < M->nelem; ele++)
        {
            for (int k = 0; k < inp->colorid_num; k++)
            {
                if (M->relems[ele] == inp->colorid[k])
                {
                    for (int p = 0; p < M->nrpts; p++)
                        t[(M->elems[M->nrpts * ele + p]) - 1] = inp->thick_r[k];
                    break;
                }
            }
        }
    }
    // applied the label{color} mask
    if (inp->used_cmask==1){
        for (int ele = 0; ele < M->nelem; ele++)
        {
            for (int k = 0; k < inp->label_num; k++)
            {
                if (M->Melem[ele] == inp->label[k])
                {
                    for (int p = 0; p < M->nrpts; p++)
                        t[(M->elems[M->nrpts * ele + p]) - 1] = inp->thick_l[k];
                    break;
                }
            }
        }
    }
    M->t = t;
    printf("* thickness arr calculated by considering regional and material masks\n");
    return e;
}
int calctriyoung(mesh *M, input *inp)
{
    int e = 0;
    static double *young;
    young = malloc((size_t)M->nelem * sizeof(*young));
    // applied region mask
    if (inp->used_rmask==1){
        for (int ele = 0; ele < M->nelem; ele++)
        {
            for (int k = 0; k < inp->colorid_num; k++)
            {
                if (M->relems[ele] == inp->colorid[k])
                {
                    young[ele] = inp->young_r[k];
                    break;
                }
            }
        }
    }
    // applied the label{color} mask
    if (inp->used_cmask==1){
        for (int ele = 0; ele < M->nelem; ele++)
        {
            for (int k = 0; k < inp->label_num; k++)
            {
                if (M->Melem[ele] == inp->label[k])
                {
                    young[ele] = inp->young_l[k];
                    break;
                }
            }
        }
    }
    M->young = young;
    printf("* Young Modulus arr calculated by considering material mask\n");
    return e;
}
int calctripres(mesh *M,mesh *M1, input *inp)
{
    int e = 0;
    static int *pres;
    pres = calloc((size_t)M->nelem, sizeof(*pres));
    if (inp->used_BCmask == 1)
    {
        // appled the BCmask
        for (int ele = 0; ele < M->nelem; ele++)
        {
            pres[ele] = M->BCmask[ele];
        }
    }
    else
    {
        // applied the regional mask
        for (int ele = 0; ele < M->nelem; ele++)
        {
            for (int k = 0; k < inp->load_region_num; k++)
            {
                if (M->relems[ele] == inp->colorid[k])
                {
                    pres[ele] = inp->load_region[k];
                    break;
                }
            }
        }
    }
    //check the fix and pressure mask overlap 
        static int *pres2;int nei;
        pres2 = calloc((size_t)M->nelem, sizeof(*pres2));
        for (int ele=0;ele<M->nelem;ele++) pres2[ele]=pres[ele];
        for (int ele=0;ele<M->nelem;ele++)
        {
            if (pres[ele]==1){
                for (int i=0;i<M->nredge;i++)
                {
                    nei = M1->esure[M->nredge*ele+i];
                    if (pres[nei]==0){
                        pres2[ele]=0;
                        break;
                    }
                }
            }
        }
        for (int ele=0;ele<M->nelem;ele++) pres[ele]=pres2[ele];
        for (int ele=0;ele<M->nelem;ele++)
        {
            if (pres[ele]==1){
                for (int i=0;i<M->nredge;i++)
                {
                    nei = M1->esure[M->nredge*ele+i];
                    if (pres[nei]==0){
                        pres2[ele]=0;
                        break;
                    }
                }
            }
        }
    M->presmask = pres2;
    printf("* A mask of elements is applied by considering both the regional mask and the input file.\n");
    return e;
}
int calctrifixb(mesh *M, input *inp)
{
    int e = 0;
    static int *fixb;
    fixb = calloc((size_t)M->nelem, sizeof(*fixb));
    if (inp->used_BCmask == 1)
    {
        // appled the BCmask
        for (int ele = 0; ele < M->nelem; ele++)
        {
            if (M->BCmask[ele] == 0)
            {
                fixb[ele] = 1;
            }
            else if (M->BCmask[ele] == 1)
            {
                fixb[ele] = 0;
            }
            else
            {
                fprintf(stderr, "ERROR: there is problem in the line %d of the BCmask file.\n", ele);
                fprintf(stderr, "the value of the BCmask should be 0 or 1 but here is %d\n", M->BCmask[ele]);
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        // applied the regional  mask
        for (int ele = 0; ele < M->nelem; ele++)
        {
            for (int k = 0; k < inp->fix_region_num; k++)
            {
                if (M->relems[ele] == inp->colorid[k])
                {
                    fixb[ele] = inp->fix_region[k];
                    break;
                }
            }
        }
    }
    M->fixbmask = fixb;
    printf("* A mask of fixed elements by considering both regional mask and the input file.\n");
    return e;
}
void NeoHokfunc(FILE *fptr, input *inp)
{
    fprintf(fptr, "\t\t<material id=\"1\" name=\"Material1\" type=\"prestrain elastic\">\n");
    fprintf(fptr, "\t\t\t<density>%lf</density>\n", inp->ro);
    fprintf(fptr, "\t\t\t<elastic type=\"neo-Hookean\">\n");
    fprintf(fptr, "\t\t\t\t<density>%lf</density>\n", inp->ro);
    fprintf(fptr, "\t\t\t\t<E type=\"map\">map_E</E>\n");
    // fprintf(fptr,"\t\t\t\t<E>2e+07</E>\n");
    fprintf(fptr, "\t\t\t\t<v>%lf</v>\n", inp->pois);
    fprintf(fptr, "\t\t\t</elastic>\n");
    fprintf(fptr, "\t\t\t<prestrain type=\"prestrain gradient\">\n");
    fprintf(fptr, "\t\t\t\t<ramp>1</ramp>\n");
    fprintf(fptr, "\t\t\t\t<F0>1,0,0,0,1,0,0,0,1</F0>\n");
    fprintf(fptr, "\t\t\t</prestrain>\n");
    fprintf(fptr, "\t\t</material>\n");
}
void Elementfunc(FILE *fptr, mesh *M)
{
    // write elements field
    fprintf(fptr, "\t\t<Elements type=\"%s%d\" name=\"Part1\">\n", M->type, M->nrpts);
    for (int ele = 0; ele < M->nelem; ele++)
    {
        fprintf(fptr, "\t\t\t<elem id=\"%d\">", ele + 1);
        int ip = 0;
        while (ip < M->nrpts)
        {
            fprintf(fptr, "%d", M->elems[M->nrpts * ele + ip]);
            ip++;
            if (ip < M->nrpts)
                fprintf(fptr, ",");
        }
        fprintf(fptr, "</elem>\n");
    }
    fprintf(fptr, "\t\t</Elements>\n");
    // write surface field for all masks file
    fprintf(fptr, "\t\t<Surface name=\"FixedShellDisplacement1\">\n");
    int ifix = 0;
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->fixbmask[ele] != 1)
            continue;
        ifix++;
        fprintf(fptr, "\t\t\t<%s%d id=\"%d\">", M->type, M->nrpts, ifix);
        int ip = 0;
        while (ip < M->nrpts)
        {
            fprintf(fptr, "%d", M->elems[M->nrpts * ele + ip]);
            ip++;
            if (ip < M->nrpts)
                fprintf(fptr, ",");
        }
        fprintf(fptr, "</%s%d>\n", M->type, M->nrpts);
    }
    fprintf(fptr, "\t\t</Surface>\n");
    fprintf(fptr, "\t\t<Surface name=\"PressureLoad1\">\n");
    int ipres = 0;
    for (int ele = 0; ele < M->nelem; ele++)
    {
        if (M->presmask[ele] != 1)
            continue;
        ipres++;
        fprintf(fptr, "\t\t\t<%s%d id=\"%d\">", M->type, M->nrpts, ipres);
        int ip = 0;
        while (ip < M->nrpts)
        {
            fprintf(fptr, "%d", M->elems[M->nrpts * ele + ip]);
            ip++;
            if (ip < M->nrpts)
                fprintf(fptr, ",");
        }
        fprintf(fptr, "</%s%d>\n", M->type, M->nrpts);
    }
    fprintf(fptr, "\t\t</Surface>\n");
}
void shellthickfunc(FILE *fptr, mesh *M)
{
    fprintf(fptr, "\t\t<ElementData type=\"shell thickness\" elem_set=\"Part1\">\n");
    for (int ele = 0; ele < M->nelem; ele++)
    {
        fprintf(fptr, "\t\t\t<e lid=\"%d\">", ele + 1);
        int ip = 0;
        while (ip < M->nrpts)
        {
            fprintf(fptr, "%lf", M->t[(M->elems[M->nrpts * ele + ip]) - 1]);
            ip++;
            if (ip < M->nrpts)
                fprintf(fptr, ",");
        }
        fprintf(fptr, "</e>\n");
    }
    fprintf(fptr, "\t\t</ElementData>\n");
}
void pres_stepfunc(FILE *fptr)
{
    fprintf(fptr, "\t\t\t\t<Control>\n");
    fprintf(fptr, "\t\t\t\t\t<analysis>STATIC</analysis>\n");
    fprintf(fptr, "\t\t\t\t\t<time_steps>10</time_steps>\n");
    fprintf(fptr, "\t\t\t\t\t<step_size>0.1</step_size>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_zero_state>0</plot_zero_state>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_range>0,-1</plot_range>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_level>PLOT_MAJOR_ITRS</plot_level>\n");
    fprintf(fptr, "\t\t\t\t\t<output_level>OUTPUT_MAJOR_ITRS</output_level>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_stride>1</plot_stride>\n");
    fprintf(fptr, "\t\t\t\t\t<output_stride>1</output_stride>\n");
    fprintf(fptr, "\t\t\t\t\t<adaptor_re_solve>1</adaptor_re_solve>\n");
    fprintf(fptr, "\t\t\t\t\t<time_stepper type=\"default\">\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_retries>10</max_retries>\n");
    fprintf(fptr, "\t\t\t\t\t\t<opt_iter>100</opt_iter>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtmin>0</dtmin>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtmax>0.1</dtmax>\n");
    fprintf(fptr, "\t\t\t\t\t\t<aggressiveness>0</aggressiveness>\n");
    fprintf(fptr, "\t\t\t\t\t\t<cutback>0.5</cutback>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtforce>0</dtforce>\n");
    fprintf(fptr, "\t\t\t\t\t</time_stepper>\n");
    fprintf(fptr, "\t\t\t\t\t<solver type=\"solid\">\n");
    fprintf(fptr, "\t\t\t\t\t\t<symmetric_stiffness>non-symmetric</symmetric_stiffness>\n");
    fprintf(fptr, "\t\t\t\t\t\t<equation_scheme>staggered</equation_scheme>\n");
    fprintf(fptr, "\t\t\t\t\t\t<equation_order>default</equation_order>\n");
    fprintf(fptr, "\t\t\t\t\t\t<optimize_bw>0</optimize_bw>\n");
    fprintf(fptr, "\t\t\t\t\t\t<lstol>0.9</lstol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<lsmin>0.01</lsmin>\n");
    fprintf(fptr, "\t\t\t\t\t\t<lsiter>5</lsiter>\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_refs>30</max_refs>\n");
    fprintf(fptr, "\t\t\t\t\t\t<check_zero_diagonal>0</check_zero_diagonal>\n");
    fprintf(fptr, "\t\t\t\t\t\t<zero_diagonal_tol>0</zero_diagonal_tol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<force_partition>0</force_partition>\n");
    fprintf(fptr, "\t\t\t\t\t\t<reform_each_time_step>1</reform_each_time_step>\n");
    fprintf(fptr, "\t\t\t\t\t\t<reform_augment>0</reform_augment>\n");
    fprintf(fptr, "\t\t\t\t\t\t<diverge_reform>1</diverge_reform>\n");
    fprintf(fptr, "\t\t\t\t\t\t<min_residual>1e-15</min_residual>\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_residual>0</max_residual>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtol>1e-05</dtol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<etol>0.0001</etol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<rtol>0</rtol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<rhoi>0</rhoi>\n");
    fprintf(fptr, "\t\t\t\t\t\t<alpha>1</alpha>\n");
    fprintf(fptr, "\t\t\t\t\t\t<beta>0.25</beta>\n");
    fprintf(fptr, "\t\t\t\t\t\t<gamma>0.5</gamma>\n");
    fprintf(fptr, "\t\t\t\t\t\t<logSolve>0</logSolve>\n");
    fprintf(fptr, "\t\t\t\t\t\t<arc_length>0</arc_length>\n");
    fprintf(fptr, "\t\t\t\t\t\t<arc_length_scale>0</arc_length_scale>\n");
    fprintf(fptr, "\t\t\t\t\t\t<qn_method type=\"full Newton\"/>\n");
    fprintf(fptr, "\t\t\t\t\t</solver>\n");
    fprintf(fptr, "\t\t\t\t</Control>\n");
    fprintf(fptr, "\t\t\t\t<Constraints>\n");
    fprintf(fptr, "\t\t\t\t\t<constraint name=\"Prestrain1\" type=\"prestrain\">\n");
    fprintf(fptr, "\t\t\t\t\t\t<update>1</update>\n");
    fprintf(fptr, "\t\t\t\t\t\t<tolerance>0.1</tolerance>\n");
    fprintf(fptr, "\t\t\t\t\t\t<min_iters>3</min_iters>\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_iters>-1</max_iters>\n");
    fprintf(fptr, "\t\t\t\t\t</constraint>\n");
    fprintf(fptr, "\t\t\t\t</Constraints>\n");
}
void norm_stepfunc(FILE *fptr)
{
    fprintf(fptr, "\t\t\t\t<Control>\n");
    fprintf(fptr, "\t\t\t\t\t<analysis>STATIC</analysis>\n");
    fprintf(fptr, "\t\t\t\t\t<time_steps>10</time_steps>\n");
    fprintf(fptr, "\t\t\t\t\t<step_size>0.1</step_size>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_zero_state>0</plot_zero_state>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_range>0,-1</plot_range>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_level>PLOT_MAJOR_ITRS</plot_level>\n");
    fprintf(fptr, "\t\t\t\t\t<output_level>OUTPUT_MAJOR_ITRS</output_level>\n");
    fprintf(fptr, "\t\t\t\t\t<plot_stride>1</plot_stride>\n");
    fprintf(fptr, "\t\t\t\t\t<output_stride>1</output_stride>\n");
    fprintf(fptr, "\t\t\t\t\t<adaptor_re_solve>1</adaptor_re_solve>\n");
    fprintf(fptr, "\t\t\t\t\t<time_stepper type=\"default\">\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_retries>5</max_retries>\n");
    fprintf(fptr, "\t\t\t\t\t\t<opt_iter>11</opt_iter>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtmin>0</dtmin>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtmax>0.1</dtmax>\n");
    fprintf(fptr, "\t\t\t\t\t\t<aggressiveness>0</aggressiveness>\n");
    fprintf(fptr, "\t\t\t\t\t\t<cutback>0.5</cutback>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtforce>0</dtforce>\n");
    fprintf(fptr, "\t\t\t\t\t</time_stepper>\n");
    fprintf(fptr, "\t\t\t\t\t<solver type=\"solid\">\n");
    fprintf(fptr, "\t\t\t\t\t\t<symmetric_stiffness>non-symmetric</symmetric_stiffness>\n");
    fprintf(fptr, "\t\t\t\t\t\t<equation_scheme>staggered</equation_scheme>\n");
    fprintf(fptr, "\t\t\t\t\t\t<equation_order>default</equation_order>\n");
    fprintf(fptr, "\t\t\t\t\t\t<optimize_bw>0</optimize_bw>\n");
    fprintf(fptr, "\t\t\t\t\t\t<lstol>0.9</lstol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<lsmin>0.01</lsmin>\n");
    fprintf(fptr, "\t\t\t\t\t\t<lsiter>5</lsiter>\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_refs>15</max_refs>\n");
    fprintf(fptr, "\t\t\t\t\t\t<check_zero_diagonal>0</check_zero_diagonal>\n");
    fprintf(fptr, "\t\t\t\t\t\t<zero_diagonal_tol>0</zero_diagonal_tol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<force_partition>0</force_partition>\n");
    fprintf(fptr, "\t\t\t\t\t\t<reform_each_time_step>1</reform_each_time_step>\n");
    fprintf(fptr, "\t\t\t\t\t\t<reform_augment>0</reform_augment>\n");
    fprintf(fptr, "\t\t\t\t\t\t<diverge_reform>1</diverge_reform>\n");
    fprintf(fptr, "\t\t\t\t\t\t<min_residual>1e-20</min_residual>\n");
    fprintf(fptr, "\t\t\t\t\t\t<max_residual>0</max_residual>\n");
    fprintf(fptr, "\t\t\t\t\t\t<dtol>0.001</dtol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<etol>0.01</etol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<rtol>0</rtol>\n");
    fprintf(fptr, "\t\t\t\t\t\t<rhoi>-2</rhoi>\n");
    fprintf(fptr, "\t\t\t\t\t\t<alpha>1</alpha>\n");
    fprintf(fptr, "\t\t\t\t\t\t<beta>0.25</beta>\n");
    fprintf(fptr, "\t\t\t\t\t\t<gamma>0.5</gamma>\n");
    fprintf(fptr, "\t\t\t\t\t\t<logSolve>0</logSolve>\n");
    fprintf(fptr, "\t\t\t\t\t\t<arc_length>0</arc_length>\n");
    fprintf(fptr, "\t\t\t\t\t\t<arc_length_scale>0</arc_length_scale>\n");
    fprintf(fptr, "\t\t\t\t\t\t<qn_method type=\"full Newton\"/>\n");
    fprintf(fptr, "\t\t\t\t\t</solver>\n");
    fprintf(fptr, "\t\t\t\t</Control>\n");
}
int febmkr(char *dir, char *name, int step, mesh *M, input *inp)
{
    int e = 0;
    // check the start ID element
    if (checkEIDS(M->elems) != 1)
    {
        fprintf(stderr, "ERROR: The element ID should start from 1 for febmkr function!\n");
        return -1;
    }
    char num[10];
    sprintf(num, "%d", step);
    char path[500];
    strcpy(path, dir);
    strcat(path, name);
    strcat(path, "_");
    strcat(path, num);
    strcat(path, ".feb");
    char command[500];
    strcpy(command, "rm ");
    strcat(command, path);
    /* define File pointer:*/
    FILE *fptr;
    fptr = calloc(1, sizeof(*fptr));
    /* Opening File */
    fptr = fopen(path, "w");
    if (fptr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
        return -1;
    }
    /*write feb file : */
    fprintf(fptr, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
    fprintf(fptr, "<febio_spec version=\"4.0\">\n");
    fprintf(fptr, "\t<Module type=\"solid\"/>\n");
    fprintf(fptr, "\t<Globals>\n");
    fprintf(fptr, "\t\t<Constants>\n");
    fprintf(fptr, "\t\t\t<T>0</T>\n\t\t\t<P>0</P>\n\t\t\t<R>0</R>\n\t\t\t<Fc>0</Fc>\n");
    fprintf(fptr, "\t\t</Constants>\n");
    fprintf(fptr, "\t</Globals>\n");
    fprintf(fptr, "\t<Material>\n");
    NeoHokfunc(fptr, inp);
    fprintf(fptr, "\t</Material>\n");
    fprintf(fptr, "\t<Mesh>\n");
    fprintf(fptr, "\t\t<Nodes name=\"Object1\">\n");
    for (int i = 0; i < M->npoin; i++)
    {
        fprintf(fptr, "\t\t\t<node id=\"%d\">%lf,%lf,%lf</node>\n", i + 1, M->ptxyz[3 * i],
                M->ptxyz[3 * i + 1], M->ptxyz[3 * i + 2]);
    }
    fprintf(fptr, "\t\t</Nodes>\n");
    Elementfunc(fptr, M);
    fprintf(fptr, "\t\t<PartList name=\"map_E\">Part1</PartList>\n");
    fprintf(fptr, "\t</Mesh>\n");
    fprintf(fptr, "\t<MeshDomains>\n");
    fprintf(fptr, "\t\t<ShellDomain name=\"Part1\" mat=\"Material1\">\n");
    fprintf(fptr, "\t\t\t<shell_thickness>0</shell_thickness>\n");
    fprintf(fptr, "\t\t</ShellDomain>\n");
    fprintf(fptr, "\t</MeshDomains>\n");
    fprintf(fptr, "\t<MeshData>\n");
    shellthickfunc(fptr, M);
    fprintf(fptr, "\t\t<ElementData name=\"map_E\" elem_set=\"Part1\">\n");
    for (int ele = 0; ele < M->nelem; ele++)
        fprintf(fptr, "\t\t\t<e lid=\"%d\">%lf</e>\n", ele + 1, M->young[ele]);
    fprintf(fptr, "\t\t</ElementData>\n");
    fprintf(fptr, "\t</MeshData>\n");
    fprintf(fptr, "\t<Boundary>\n");
    fprintf(fptr, "\t\t<bc name=\"FixedDisplacement1\" node_set=\"@surface:FixedShellDisplacement1\" type=\"zero displacement\">\n");
    fprintf(fptr, "\t\t\t<x_dof>1</x_dof>\n");
    fprintf(fptr, "\t\t\t<y_dof>1</y_dof>\n");
    fprintf(fptr, "\t\t\t<z_dof>1</z_dof>\n");
    fprintf(fptr, "\t\t</bc>\n");
    fprintf(fptr, "\t\t<bc name=\"FixedShellDisplacement1\" node_set=\"@surface:FixedShellDisplacement1\" type=\"zero shell displacement\">\n");
    fprintf(fptr, "\t\t\t<sx_dof>1</sx_dof>\n");
    fprintf(fptr, "\t\t\t<sy_dof>1</sy_dof>\n");
    fprintf(fptr, "\t\t\t<sz_dof>1</sz_dof>\n");
    fprintf(fptr, "\t\t</bc>\n");
    fprintf(fptr, "\t\t<bc name=\"ZeroRotation3\" node_set=\"@surface:FixedShellDisplacement1\" type=\"zero rotation\">\n");
    fprintf(fptr, "\t\t\t<u_dof>1</u_dof>\n");
    fprintf(fptr, "\t\t\t<v_dof>1</v_dof>\n");
    fprintf(fptr, "\t\t\t<w_dof>1</w_dof>\n");
    fprintf(fptr, "\t\t</bc>\n");
    fprintf(fptr, "\t</Boundary>\n");
    fprintf(fptr, "\t<Loads>\n");
    fprintf(fptr, "\t\t<surface_load name=\"Pressure1\" surface=\"PressureLoad1\" type=\"pressure\">\n");
    fprintf(fptr, "\t\t\t<pressure lc=\"1\">%lf</pressure>\n", inp->pres);
    fprintf(fptr, "\t\t\t<symmetric_stiffness>0</symmetric_stiffness>\n");
    fprintf(fptr, "\t\t\t<linear>0</linear>\n");
    fprintf(fptr, "\t\t\t<shell_bottom>0</shell_bottom>\n");
    fprintf(fptr, "\t\t</surface_load>\n");
    fprintf(fptr, "\t</Loads>\n");
    fprintf(fptr, "\t<Constraints>\n");
    fprintf(fptr, "\t</Constraints>\n");
    fprintf(fptr, "\t<Step>\n");
    fprintf(fptr, "\t\t<step id=\"1\" name=\"Step01\">\n");
    pres_stepfunc(fptr);
    fprintf(fptr, "\t\t</step>\n");
    fprintf(fptr, "\t\t<step id=\"2\" name=\"Step02\">\n");
    norm_stepfunc(fptr);
    fprintf(fptr, "\t\t</step>\n");
    fprintf(fptr, "\t</Step>\n");
    fprintf(fptr, "\t<LoadData>\n");
    fprintf(fptr, "\t\t<load_controller id=\"1\" name=\"LC1\" type=\"loadcurve\">\n");
    fprintf(fptr, "\t\t\t<interpolate>LINEAR</interpolate>\n");
    fprintf(fptr, "\t\t\t<extend>CONSTANT</extend>\n");
    fprintf(fptr, "\t\t\t<points>\n");
    fprintf(fptr, "\t\t\t\t<pt>0,0</pt>\n");
    fprintf(fptr, "\t\t\t\t<pt>1,1</pt>\n");
    fprintf(fptr, "\t\t\t\t<pt>2,%lf</pt>\n", inp->ultipres / inp->pres);
    fprintf(fptr, "\t\t\t</points>\n");
    fprintf(fptr, "\t\t</load_controller>\n");
    fprintf(fptr, "\t</LoadData>\n");
    fprintf(fptr, "\t<Output>\n");
    fprintf(fptr, "\t\t<plotfile type=\"febio\">\n");
    fprintf(fptr, "\t\t\t<var type=\"displacement\"/>\n");
    fprintf(fptr, "\t\t\t<var type=\"relative volume\"/>\n");
    fprintf(fptr, "\t\t\t<var type=\"shell strain\"/>\n");
    fprintf(fptr, "\t\t\t<var type=\"shell thickness\"/>\n");
    fprintf(fptr, "\t\t\t<var type=\"stress\"/>\n");
    fprintf(fptr, "\t\t</plotfile>\n");
    fprintf(fptr, "\t</Output>\n");
    if(inp->print_st==1){
        fprintf(fptr, "\t<Output>\n");
        fprintf(fptr, "\t\t<logfile>\n");
        fprintf(fptr, "\t\t\t<element_data data=\"sx;sy;sz;sxy;syz;sxz\" name=\"element stresses\"> </element_data>\n");
        fprintf(fptr, "\t\t</logfile>\n");
        fprintf(fptr, "\t</Output>\n");
    }
    fprintf(fptr, "</febio_spec>\n");
    if (fclose(fptr) == EOF)
    {
        // If fclose returns EOF, it means there was an error closing the file
        printf("Error closing %s\n", path);
        return -1;
    }
    printf("* wrote %s in the feb format!\n", path);
    return e;
}