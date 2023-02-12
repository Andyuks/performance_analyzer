/* data_struct.h*/
#define ERROR_FILE 1
#define ERROR_MEASURES 2
#define ERROR_DIR 3

/* structure of system*/
struct system_t
{
	unsigned int * cpus; /* cpu amount */
	double * runtime; /* average time per processor */
	double * speedup; /* speed-up achieved per processor */
	double * efficiency; /* efficiency per processor */

	unsigned int ncpus; /* number of processors */
	unsigned int nruns; /* number of runs */
	unsigned int hasheader; /* 0: dataset file lacks header, 1: dataset file has header */
};

void initialize_system(struct system_t * system);
void free_system(struct system_t * system);

