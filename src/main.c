/*******************************
SCP EXTRA0: PERFORMANCE ANALYZER 
********************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#ifndef DATA_STRUCT_H
	#define DATA_STRUCT_H
	#include "../include/data_struct.h"
#endif /* DATA_STRUCT_H */

#include "../include/aux_funs.h"

void parse_arguments(int argc, char *argv[], char * filename);

/* main program*/
int main (int argc, char * argv[]){
	/* declare variables */
	struct system_t system;
	char filename[256];
	int ret;

	printf("\n\033[1;35m****************************\033[0m\n");
	printf("\033[1;35m*** PERFORMANCE ANALYZER ***\033[0m\n");
	printf("\033[1;35m****************************\033[0m\n");

	/* parse arguments */
	parse_arguments(argc, argv, filename);
	
	/* prepare workspace with due data and measures */
	ret = set_workspace(&system, filename);
	if(ret!=0)	exit(ret);

	/* print and export results */
	print_results(&system, stdout);
	ret = produce_file(&system);
	if(ret!=0)	
    {
        free_system(&system); 
        exit(ret);
    }
	/* produce graphics */
	ret = produce_graphics_gnuplot(&system);

	/* free memory */
	free_system(&system); 
	exit(ret);
}


/* function to parse arguments */
void parse_arguments(int argc, char *argv[], char * filename)
{
	int o, long_index;
	static struct option long_options[] = {
        {"help",       no_argument,       0,  'h' },
        {"filename",      required_argument, 0,  'f' },
        {0,            0,                 0,   0  }
    };
	strcpy(filename, "./input/prueba.escalabilidad.txt"); // default filename
    
    /* Manage options */
	while ((o = getopt_long(argc, argv,":h:f:", 
                        long_options, &long_index )) != -1) 
	{
		switch(o)
		{
			case 'h':
				printf("Use: %s [OPTIONS]\n", argv[0]);
				printf(" --h, --help \t\t Help\n");
				printf(" -f, --filename=S \t\t Name of the dataset file\n");
				exit(0);
			case 'f':
				strcpy(filename, optarg);
				break;
			default:
				printf("Unknown argument option\n");
		}
	}
}
