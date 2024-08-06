#include <string.h>
#include "common.h"
#include "mystructs.h"
#include "myfuncs.h"
#include "febiofuncs.h"

int rinputf(char *dir, mesh *M1, input *inp)
{
	int e = 0;
	// Create a hash table
	HashTable table = {0};

	FILE *fprt;
	fprt = calloc(1, sizeof(*fprt));
	char path[500];
	strcpy(path, dir);
	strcat(path, "input.txt");
	fprt = fopen(path, "r");
	if (fprt == NULL)
	{
		fprintf(stderr, "ERROR: the %s dose not open.\n ", path);
	}
	char line[100], *str, key[50], value[50];
	int i = 0;
	while ((str = fgets(line, 100, fprt)))
	{
		i++;
		if (line[0] != '/')
		{
			sscanf(str, "%s %s", key, value);
			inserthash(&table, key, value);
		}
		// printf("%d\n",i);
	}
	fclose(fprt);

	// MESH:
	strcpy(M1->type, gethash(&table, "type"));
	M1->nredge = atoi(gethash(&table, "nredge"));
	M1->nrpts = atoi(gethash(&table, "nrpts"));

	// Solver
	strcpy(inp->nonlinear_FE, gethash(&table, "nonlinear_FE")); // BFGS or  full Newton
	inp->symetric_stiff = 0;

	// Neo-Hooken model
	// strcpy(inp->Mmodel,gethash(&table,"isotropic elastic")); // isotropic elastic  or  neo-Hookean or coupled Mooney-Rivlin
	static double young_l2[3] = {0, 0, 0}; //{red, yellow, white} [dyne/cm^2]
	young_l2[0] = atof(gethash(&table, "young_red"));
	young_l2[1] = atof(gethash(&table, "young_yellow"));
	young_l2[2] = atof(gethash(&table, "young_white"));
	inp->young_l = young_l2;
	static double young_r2[6] = {0, 0, 0, 0, 0, 0}; // region { remain(another aneu),diastal,parent,neck,body,dome} [dyne/cm^2]
	young_r2[0] = atof(gethash(&table, "young_remain"));
	young_r2[1] = atof(gethash(&table, "young_distal"));
	young_r2[2] = atof(gethash(&table, "young_parent"));
	young_r2[3] = atof(gethash(&table, "young_neck"));
	young_r2[4] = atof(gethash(&table, "young_body"));
	young_r2[5] = atof(gethash(&table, "young_dome"));
	inp->young_r = young_r2;
	// printf("%lf %lf %lf %lf %lf %lf\n",inp->young_r[0],inp->young_r[1],inp->young_r[2],inp->young_r[3],inp->young_r[4],inp->young_r[5]);
	inp->pois = atof(gethash(&table, "pois"));
	inp->ro = atof(gethash(&table, "ro")); //[gr/cm^3]
	inp->NJyoung = atof(gethash(&table, "NJyoung"));
	inp->incyoung = atof(gethash(&table, "incyoung"));

	// label : <red, yellow, white, cyan, rupture, remain>
	static int label2[3] = {0, 0, 0};
	label2[0] = atoi(gethash(&table, "label_red"));
	label2[1] = atoi(gethash(&table, "label_yellow"));
	label2[2] = atoi(gethash(&table, "label_white"));
	inp->label_num = 3;
	inp->label = label2;
	// printf("label : %d %d %d \n",inp->label[0],inp->label[1],inp->label[2]);

	// boundary condition:
	inp->used_BCmask = atoi(gethash(&table, "used_BCmask")); //  1 :  use mask --- 0 : used region id
	static int colorid2[6] = {0, 0, 0, 0, 0, 0};			 // region { remain(another aneu),diastal,parent,neck,body,dome}
	colorid2[0] = atoi(gethash(&table, "colorid_remain"));
	colorid2[1] = atoi(gethash(&table, "colorid_distal"));
	colorid2[2] = atoi(gethash(&table, "colorid_parent"));
	colorid2[3] = atoi(gethash(&table, "colorid_neck"));
	colorid2[4] = atoi(gethash(&table, "colorid_body"));
	colorid2[5] = atoi(gethash(&table, "colorid_dome"));
	inp->colorid_num = 6;
	inp->colorid = colorid2;

	static int fix_region2[6] = {0, 0, 0, 0, 0, 0}; // region { remain(another aneu),diastal,parent,neck,body,dome}
	fix_region2[0] = atoi(gethash(&table, "fix_remain"));
	fix_region2[1] = atoi(gethash(&table, "fix_distal"));
	fix_region2[2] = atoi(gethash(&table, "fix_parent"));
	fix_region2[3] = atoi(gethash(&table, "fix_neck"));
	fix_region2[4] = atoi(gethash(&table, "fix_body"));
	fix_region2[5] = atoi(gethash(&table, "fix_dome"));
	inp->fix_region = fix_region2;
	inp->fix_region_num = 6;
	static int load_region2[6] = {0, 0, 0, 0, 0, 0}; // region { remain(another aneu),diastal,parent,neck,body,dome}
	load_region2[0] = atoi(gethash(&table, "load_remain"));
	load_region2[1] = atoi(gethash(&table, "load_distal"));
	load_region2[2] = atoi(gethash(&table, "load_parent"));
	load_region2[3] = atoi(gethash(&table, "load_neck"));
	load_region2[4] = atoi(gethash(&table, "load_body"));
	load_region2[5] = atoi(gethash(&table, "load_dome"));
	inp->load_region = load_region2;
	inp->load_region_num = 6;
	// thickness
	static double thick_r2[6] = {0, 0, 0, 0, 0, 0}; // { remain(another aneu),diastal,parent,neck,body,dome}[cm]
	thick_r2[0] = atof(gethash(&table, "thick_remain"));
	thick_r2[1] = atof(gethash(&table, "thick_distal"));
	thick_r2[2] = atof(gethash(&table, "thick_parent"));
	thick_r2[3] = atof(gethash(&table, "thick_neck"));
	thick_r2[4] = atof(gethash(&table, "thick_body"));
	thick_r2[5] = atof(gethash(&table, "thick_dome"));
	inp->thick_r = thick_r2;
	inp->thick_r_num = 6;

	static double thick_l2[3] = {0, 0, 0}; // {red, yellow, white} [cm]
	thick_l2[0] = atof(gethash(&table, "thick_red"));
	thick_l2[1] = atof(gethash(&table, "thick_yellow"));
	thick_l2[2] = atof(gethash(&table, "thick_white"));
	inp->thick_l = thick_l2;
	inp->thick_l_num = 3;
	// pre
	inp->pres = atof(gethash(&table, "pre_stress"));
	inp->ultipres = atof(gethash(&table, "2step_pres"));

	// Free the memory allocated for the hash table
	freeTable(&table);
	return e;
}
int datafiles(void)
{
	int e = 0;
	strcpy(datafilepath[0], datadir);
	strcat(datafilepath[0], filename);
	strcat(datafilepath[0], dot);

	strcpy(datafilepath[1], datafilepath[0]);

	strcat(datafilepath[0], "flds.zfem");
	strcat(datafilepath[1], "wall");

	strcpy(datafilepath[2], datadir);
	strcat(datafilepath[2], "labels_srf.zfem");

	strcpy(datafilepath[3], datadir);
	strcat(datafilepath[3], "BCmask.txt");
	return e;
}
