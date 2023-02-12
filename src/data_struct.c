/* data_struct.c */
#include <stdlib.h>

#ifndef DATA_STRUCT_H
	#define DATA_STRUCT_H
	#include "../include/data_struct.h"
#endif /* DATA_STRUCT_H */

/* deals with the initialization of the system */
void initialize_system(struct system_t * system)
{
	system->cpus = malloc(system->ncpus * sizeof(double));
	system->runtime = malloc(system->ncpus * sizeof(double));
	system->speedup = malloc(system->ncpus * sizeof(double));
	system->efficiency = malloc(system->ncpus * sizeof(double));

}
	
/* deals with freeing the memory allocated dynamically*/
void free_system(struct system_t * system)
{
	free(system->cpus);
	free(system->runtime);
	free(system->speedup);
	free(system->efficiency);
}

/* 
abrir dos veces o llamar a todo desde misma fun??????????????????????????????????????????????????????????

check no rows or no cols?
*/