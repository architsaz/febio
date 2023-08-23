#ifndef MYLIB_H 

#define MYLIB_H 

// define genral parameters 
#define dimension  3 // dimension of domain

// decler struct
	typedef struct {
	


	}allinfo;

//headers 
	char *edit_endline_character(char *line, int buffer, FILE *fptr);
	void read_zfem(char *casename, char *path, int *npoin, int *nelem, double **ptxyz,int **elems);
	void save_esure(int npoin,int nelem,int *elems,int *esurp,int *esurp_pointer,int **esue2, int **open2);
	void save_esurp(int npoin,int nelem,int *elems,int **esurp2,int **esurp_pointer2);
    int *find_nei_elem3D(int *esurp_pointer,int *esurp,int *num_nei, int *open,int *elems, int ele, int ele_p1, int ele_p2);
    bool file_exists(char const *filename);
    void write_log(char *str,char *path);
    void reverse(char str[], int length);
    char* citoa(int num, char* str, int base);
    void report(char const *filename,int status);

// headers for febmkr function 
	void checkdir(char const *casename, char **path,char **path_surf2,char **path_label2,char **path_input2);
	void read_regionmask(char const *casename, int nelem, int npoin,int *elems, int **region_id2, int **region_idp2);
	void write_zfem_1intfield(char const *casename,int nelem,int npoin,int *elems,double *ptxyz, int *field);
	void read_input(char const *casename,char *path);
	void read_Mlable(char const *casename,char *path,int nelem, int *elems,int **Melem2); 
	void read_aneulable(char const *casename,int nelem,int **anelem2, int *Melem);
	void material(int nelem, int *elems,int npoin,int *Melem, int *esurp,int *esurp_pointer,int nloop, double **E_ele2,double **E_fele2,double **E_nod2, double **t_ele2, double **t_fele2, double **t_nod2);
	void write_zfem_1doubfield(char const *casename,int nelem,int npoin,int *elems,double *ptxyz, double *field);
	void boundary_labeling(int nelem, int *elems,int npoin, double *ptxyz, int *Melem,int *Mpoint,int *esure, int *region_id);
	void write_feb3_prestain(char const *casename, char **runpath,int nelem, int *elems,int npoin, double *ptxyz,double *t_fele,double *E_fele,int *Melem, double *st,double pres_gradual, int iter);
 	void write_feb4_prestain(char const *casename, char **runpath,int nelem, int *elems,int npoin, double *ptxyz,double *t_fele,double *E_fele,int *Melem, double *st,double pres_gradual, int iter);
 	void write_feb4_prestain_verold(char const *casename, char **runpath,int nelem, int *elems,int npoin, double *ptxyz,double *t_fele,double *E_fele,int *region_id, double *st,double pres_gradual, int iter);
 	void read_logfile_data(char const *casename,int nelem,int npoin,double *uxyz,double *stress, double *strain,int iter);
 	int check_febio_run(char const *casename,int iter);
 	double max_value(double *array, int size);

	#endif 
