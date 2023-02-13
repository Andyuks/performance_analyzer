/* aux_funs.h*/

/* builds the workspace out of the dataset */
int set_workspace(struct system_t * system, char * name);

/* prints the results obtained */
void print_results(struct system_t * system, FILE * fp);

/* produces a file with the results */
int produce_file(struct system_t * system);

/* produces graphics out of the results using gnuplot*/
int produce_graphics_gnuplot(struct system_t * system);
