/* aux_funs.c */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef DATA_STRUCT_H
	#define DATA_STRUCT_H
	#include "../include/data_struct.h"
#endif /* DATA_STRUCT_H */

#define NUM_COMANDOS_RES 6
#define NUM_COMANDOS_SUEF 13

///////////////
// Workspace //
///////////////

/* obtains quantity of cpus and runs out of the dataset */
int measure_dataset(struct system_t * system, FILE * fp)
{
	unsigned int cols=0, rows=0;
	char buf[1024];
	const char delim[5] = "\t\n ,";
	char* token;

	fseek(fp, 0, SEEK_SET); //go to beggining of fil

	/* header management */
	fgets(buf, sizeof(buf), fp);  //skip header's line in row count    
	token = strtok(buf, delim);

	if(token[0]=='#')
		system->hasheader = 1;	
	else
	{
		rows ++;
		system->hasheader = 0;
	}

	/* column management */
	while( token != NULL ) // Count tokens skipping separators
	{
		cols ++;
		token = strtok(NULL, delim);
	}

	/* row management */
	while(fgets(buf, sizeof(buf), fp) != NULL)
		rows++;

	system->ncpus = rows;
	system->nruns = cols -1;

	printf("\033[1;34m** File analysis **\033[0m\n");
	printf("\033[1;34m> CPUs:\033[0m %d\n", system->ncpus);
	printf("\033[1;34m> Runs:\033[0m %d\n", system->nruns);	

	if(system->ncpus < 1 || system->nruns < 1)
	{
		fprintf(stderr,"\033[1;31mError: no information can be obtained from the provided dataset.\033[0m\n"); //////////////////////////////////////////////////////// error check?
		return(ERROR_MEASURES);
	}
	return(0);
}

/* obtains due data out of the dataset */
void analzye_dataset(struct system_t * system, FILE * fp)
{
	unsigned int i, j;
	float time;
	char buf[1024];

	fseek(fp, 0, SEEK_SET); //go to beggining of file

	if(system->hasheader)
		fgets(buf, sizeof(buf), fp);  //skip header's line

	for(i=0; i<system->ncpus; i++)
	{
		fscanf(fp, "%d", &(system->cpus[i]));

		for(j=0; j<system->nruns; j++)
		{
			fscanf(fp, " %f", &time);
			system->runtime[i] += time;
		}

		system->runtime[i] = system->runtime[i] / system->nruns;
		system->speedup[i] = system->runtime[0] / system->runtime[i];
		system->efficiency[i] = system->speedup[i] / system->cpus[i];
	}

}

/* builds the workspace out of the dataset */
int set_workspace(struct system_t * system, char * name)
{	
	int ret;
	FILE * fp = fopen(name, "r");
	printf("\033[1;34m> Dataset location:\033[0m%s\n\n", name);
	/*check if file correctly opened*/
	if(fp == NULL)
	{
		fprintf(stderr,"\033[1;31mError: file could not be opened.\033[0m\n");
		return(ERROR_FILE);
	}

	/* measure dimmensions and load data in system */
	ret = measure_dataset(system, fp);
	if(ret != 0){
		fclose(fp);
		return(1);
	}
	initialize_system(system);
	analzye_dataset(system, fp);

	fclose(fp);
	return(0);
}




/////////////
// Results //
/////////////
/* prints the results*/
void print_results(struct system_t * system, FILE * fp)
{
	unsigned int i;
	fprintf(fp, "#procs\truntime\t\tspeedup\t\tefficiency\n");
	fprintf(fp, "#=====\t=======\t\t=======\t\t==========\n");

	for(i=0; i < system->ncpus; i++)
	{
		fprintf(fp, "%d\t", system->cpus[i]);
		fprintf(fp, "%f\t", system->runtime[i]);
		fprintf(fp, "%f\t", system->speedup[i]);
		fprintf(fp, "%f\n", system->efficiency[i]);
	}
}


int dirmanagement(){
	DIR* dir = opendir("./out");
	int ret=0;
	if (dir) { /* directory exists */
		closedir(dir);
	} else if (ENOENT == errno) { /* Directory does not exist. */
		if((mkdir("./out", 0750))!=0)
		{
			fprintf(stderr,"\033[1;31mError: output directory could not be created.\033[0m\n");
			ret = ERROR_DIR;
		} 
		
	} else { /* opendir() failed for some other reason. */
		fprintf(stderr,"\033[1;31mError: output directory could not be opened.\033[0m\n");
		ret = ERROR_DIR;
	}
	return(ret);
}


/* produces a file with the results */
int produce_file(struct system_t * system)
{
	int dir;
	FILE * fileruntime;

	/* check if dir exists */
	dir = dirmanagement();
	if(dir!=0) return(dir);

	/*open file and check if file correctly opened*/
	fileruntime = fopen("./out/results.txt", "w");
	if(fileruntime == NULL)
	{
		fprintf(stderr,"\033[1;31mError: result file could not be opened/created.\033[0m\n");
		return(ERROR_FILE);
	}

	/* write results and close file */
	print_results(system, fileruntime);
	fclose(fileruntime);
	return(0);
}

/* produces graphics using gnuplot*/
void produce_graphics_gnuplot(struct system_t * system)
{
	int i;

	char * results_com[] = 
		{
			"set title \"Exec. time\"",
			"set ylabel \"Runtime (ms)\"",
			"set xlabel \"Processors (int)\"",
			"set terminal png",
			"set output \"./imgs/runtime.png\"",
			"plot \"./out/results.txt\" u 1:2 notitle w lp"
		};

	char * suef_com[] = 
		{
			"set title \"Speed up and Efficiency\"",
			"set ylabel \"Speed up (fl)\"",
			"set y2label \"Efficiency (fl)\"",
			"set xlabel \"Processors (int)\"",
            "set y2range [0:100]",
            "set format y2 \"%g%%\"",
            "set y2tics nomirror",
            "set logscale x",
            "set logscale y",
            "set key bottom right box lt -1 lw 2",
			"set terminal png",
			"set output \"./imgs/speedup_efficiency.png\"",
			"plot \"./out/results.txt\" u 1:3 title \"speed-up\" axes x1y1 w lp, \"./out/results.txt\" u 1:(100*$4) title \"efficiency\" axes x1y2 w lp"
		};

	/* Create poen file to execute gnuplot*/
    FILE * window = popen("gnuplot -persist", "w");

    /* Execute gnuplot commands one by one */
    for (i=0; i<NUM_COMANDOS_RES; i++){
		fprintf(window, "%s \n", results_com[i]);
	}

	/* Execute gnuplot commands one by one */
    for (i=0; i<NUM_COMANDOS_SUEF; i++)
	{
		fprintf(window, "%s \n", suef_com[i]);
	}
}

