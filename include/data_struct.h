/* data_struct.h*/
#define ERROR_FILE 1
#define ERROR_DIR 2
#define ERROR_FORMAT 2

/* structure of the system*/
struct system_t
{
	unsigned int * cpus; /* cpu amount */
	double * runtime; /* average runtime per processor */
	double * speedup; /* speed up achieved per processor */
	double * efficiency; /* efficiency achieved per processor */

	unsigned int ncpus; /* number of processors */
	unsigned int hasheader; /* 0: dataset file lacks header, 1: dataset file has header */
};

// data_struct.c functions
void initialize_system(struct system_t * system);
void free_system(struct system_t * system);

