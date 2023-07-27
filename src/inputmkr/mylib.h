#ifndef MYLIB_H 

#define MYLIB_H 

// define genral parameters 
#define dimension  3 // dimension of domain

// decler struct
	typedef struct {
	


	}allinfo;

//headers 
	char *edit_endline_character(char *line, int buffer, FILE *fptr);
	void read_zfem(char *filename, int *npoin, int *nelem, double **ptxyz,int **elems);
	void build_path (char const *casename,char **path_opensurf_achit, char **path_labels, char **path_input);
	void save_esure(int npoin,int nelem,int *elems,int *esurp,int *esurp_pointer,int **esue2, int **open2);
	void save_esurp(int npoin,int nelem,int *elems,int **esurp2,int **esurp_pointer2);
    int *find_nei_elem3D(int *esurp_pointer,int *esurp,int *num_nei, int *open,int *elems, int ele, int ele_p1, int ele_p2);
    void write_labels(int nelem, int *open, char *path);
    void clustering_hole(int nelem,int *open, int *esure, int *elems, double *ptxyz, int **boundary1,double **centers1, int *num_bound1);
    bool file_exists(const char *filename);
    void write_input(int num_bound, double *centers, char *filename, char const	*casename);
    void makedir(char const *casename);
    void read_Generalinput(void);
 

#endif 
