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

#define NUM_COMMANDS_RUNTIME 7
#define NUM_COMMANDS_SUEF 13

///////////////
// Workspace //
///////////////

/* obtains quantity of cpus and runs out of the dataset */
int measure_dataset(struct system_t * system, FILE * fp)
{
	char buf[1024];
	const char delim[5] = "\t\n ,";
	char* token;
    system->ncpus = 0;
    
    /* Check if file is empty */
    fseek(fp, 0, SEEK_END); // Go to end of file
    if (ftell(fp) == 0)
    {
        fprintf(stderr,"\033[1;31mError: file is empty.\033[0m\n");
		return(ERROR_FORMAT);           
    }

	fseek(fp, 0, SEEK_SET); // Go to beggining of file

	/* Header management */
	fgets(buf, sizeof(buf), fp);
	token = strtok(buf, delim);
    if(token!=NULL) // Get if dataset has data in first line
    {
	    if(token[0]=='#') // Get if file has header
		    system->hasheader = 1;	
	    else
	    {
		    system->ncpus ++;
		    system->hasheader = 0;
	    }
    }
    else
    {
        fprintf(stderr,"\033[1;31mError: file does not have due format.\033[0m\n");
        return(ERROR_FORMAT);
    }

	/* Row management */
	while(fgets(buf, sizeof(buf), fp) != NULL)
		system->ncpus++;

	if(system->ncpus==0)
	{
		fprintf(stderr,"\033[1;31mError: no information can be obtained from the provided dataset.\033[0m\n");
		return(ERROR_FORMAT);
	}

    /* Print processor number */
	printf("\033[1;34m> CPUs:\033[0m %d\n\n", system->ncpus);
	return(0);
}

/* obtains due data out of the dataset */
void analyze_dataset(struct system_t * system, FILE * fp)
{
	unsigned int i, runs;
	char buf[1024];
	const char delim[5] = "\t\n ,";
	char* token;


	fseek(fp, 0, SEEK_SET); // Go to beggining of file

	if(system->hasheader)
		fgets(buf, sizeof(buf), fp);  // Skip header's line

	for(i=0; i<system->ncpus; i++)
	{
        /* Obtain cpu number */
        fgets(buf, sizeof(buf), fp);
        token = strtok(buf, delim);
        system->cpus[i] = atoi(token);

        /* prepare variables */
        token = strtok(NULL, delim);
        runs = 0;

		while( token != NULL ) 
		{            
            /* Obtain run data */            
            system->runtime[i] +=  strtod(token, NULL);
            runs ++;
            token = strtok(NULL, delim);
		}

        /* Save due data */
        if(runs==0) // case no data
        {
            system->runtime[i] = 0;
		    system->speedup[i] = 0;
		    system->efficiency[i] = 0;
        }
        else{ // case there is data
		    system->runtime[i] = system->runtime[i] / runs;
		    system->speedup[i] = system->runtime[0] / system->runtime[i];
		    system->efficiency[i] = system->speedup[i] / system->cpus[i];
        }
	}
}

/* builds the workspace out of the dataset */
int set_workspace(struct system_t * system, char * name)
{	
	int ret;
	FILE * fp = fopen(name, "r");
	printf("\033[1;34m> Dataset location:\033[0m%s\n\n", name);

	/* Check if file has been correctly opened*/
	if(fp == NULL)
	{
		fprintf(stderr,"\033[1;31mError: file could not be opened.\033[0m\n");
		return(ERROR_FILE);
	}

	/* Measure dimmensions and load data in system */
	ret = measure_dataset(system, fp);
	if(ret != 0){
		fclose(fp);
		return(ret);
	}
	initialize_system(system);
	analyze_dataset(system, fp);

	fclose(fp);
	return(0);
}




/////////////
// Results //
/////////////
/* Prints the results obtained */
void print_results(struct system_t * system, FILE * fp)
{
	unsigned int i;
    /* print header */
	fprintf(fp, "#procs\truntime\t\tspeedup\t\tefficiency\n");
	fprintf(fp, "#=====\t=======\t\t=======\t\t==========\n");

    /* print results */
	for(i=0; i < system->ncpus; i++)
	{
        if( (system->runtime[i] != 0) && (system->speedup[i] != 0) && (system->efficiency[i] != 0)) // get if there is data
        {
		    fprintf(fp, "%d\t", system->cpus[i]);
		    fprintf(fp, "%f\t", system->runtime[i]);
		    fprintf(fp, "%f\t", system->speedup[i]);
		    fprintf(fp, "%f\n", system->efficiency[i]);
        }
	}
}

/* Gets if a directory exists and has due permissions, as well as create it if it does not exist */
int dirmanagement(char * dirname){
	DIR* dir = opendir(dirname);
	int ret=0;
	if (dir) { /* Directory exists */
		closedir(dir);
	} else if (ENOENT == errno) { /* Directory does not exist. */
		if((mkdir(dirname, 0750))!=0)
		{
			fprintf(stderr,"\033[1;31mError: output directory could not be created.\033[0m\n");
			ret = ERROR_DIR;
		} 
		
	} else { /* opendir() failed for some other reason. */
		fprintf(stderr,"\033[1;31mError: directory could not be opened.\033[0m\n");
		ret = ERROR_DIR;
	}
	return(ret);
}


/* Produces a file with the results obtained */
int produce_file(struct system_t * system)
{
	int retdir;
	FILE * fileresults;

	/* Check if directory ./out exists */
	retdir = dirmanagement("./out");
	if(retdir!=0) return(retdir);

	/* Open file and check if file has been correctly opened*/
	fileresults = fopen("./out/results.txt", "w");
	if(fileresults == NULL)
	{
		fprintf(stderr,"\033[1;31mError: result file could not be opened/created.\033[0m\n");
		return(ERROR_FILE);
	}

	/* Write results and close file */
	print_results(system, fileresults);
	fclose(fileresults);
	return(0);
}

/* produces graphics using gnuplot*/
int produce_graphics_gnuplot(struct system_t * system)
{
	int i, retdir;
    /* Commands for runtime plot */
	char * runtime_com[] = 
		{
			"set title \"Exec. time\"",
			"set ylabel \"Runtime (ms)\"",
			"set xlabel \"Processors (int)\"",
            "set logscale x",
			"set terminal png",
			"set output \"./imgs/runtime.png\"",
			"plot \"./out/results.txt\" u 1:2 notitle w lp"
		};

    /* Commands for speed up and efficiency plot */
	char * suef_com[] = 
		{
			"set title \"Speed up and Efficiency\"",
			"set ylabel \"Speed up\"",
			"set y2label \"Efficiency\"",
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

	/* Check if directory ./imgs exists */
	retdir = dirmanagement("./imgs");
	if(retdir!=0) return(retdir);


    /* Execute gnuplot commands one by one (runtime)*/
    for (i=0; i<NUM_COMMANDS_RUNTIME; i++){
		fprintf(window, "%s \n", runtime_com[i]);
	}

	/* Execute gnuplot commands one by one (speed up and efficiency)*/
    for (i=0; i<NUM_COMMANDS_SUEF; i++)
	{
		fprintf(window, "%s \n", suef_com[i]);
	}
    return(0);
}


