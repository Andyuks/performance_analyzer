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
#define NUM_COMANDOS_SUEF 10

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
	if(buf[0]=='#')
		system->hasheader = 1;	
	else
	{
		rows ++;
		system->hasheader = 0;
	}
	/* column management */
	token = strtok(buf, delim);
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
	FILE * windowres;
    FILE * windowsuef;
	int i;

	char * results[] = 
		{
			"set title \"Exec. time\"",
			"set ylabel \"Tex (ms)\"",
			"set xlabel \"Processors (int)\"",
			"set terminal png",
			"set output \"./imgs/runtime.png\"",
			"plot \"./out/results.txt\" u 1:2 title \"runtime\" w points"
		};

	char * suef[] = 
		{
			"set title \"Speed-up & Efficiency\"",
			"set ylabel \"Speed-up (fl)\"",
			"set y2label \"Efficiency (fl)\"",
			"set xlabel \"Processors (int)\"",
			"set autoscale",
			"set xtic auto",
			"set ytic auto",
			"set terminal png",
			"set output \"./imgs/suef.png\"",
			"plot \"./out/results.txt\" u 1:3 title \"speed-up\" w lines",
			"plot \"./out/results.txt\" u 1:4 title \"efficiency\" w dots"
		};

	/* Create poen file to execute gnuplot and send file two plot*/
    windowres = popen ("gnuplot -persist", "w");
    windowsuef = popen ("gnuplot -persist", "w");

    /* Execute gnuplot commands one by one */
    for (i=0; i<NUM_COMANDOS_RES; i++){
		fprintf(windowres, "%s \n", results[i]);
	}

	/* Execute gnuplot commands one by one */
    for (i=0; i<NUM_COMANDOS_SUEF; i++)
	{
		fprintf(windowsuef, "%s \n", suef[i]);
	}
}


/* 
TODO: check tema sobre la marcha
min max dev in same file or two better?
paralelizar in any way
checl PNG etc
*/


/*
void produce_graphics_pbplot(struct system_t * system) 
{
	unsigned int i;
	double *pngData;

    RGBABitmapImageReference* imageref = CreateRGBABitmapImageReference();

	ScatterPlotSeries* series_r = GetDefaultScatterPlotSeriesSettings();
	series_r->xs = system->ncpus;
	series_r->ys = system->nruns;
	series_r->linearInterpolation = false;
	series_r->lineType = toVector(L"solid");
	series_r->color = CreateRGBColor(0, 0, 1);

	ScatterPlotSeries* series_idr = GetDefaultScatterPlotSeriesSettings();
	series_idr->xs = system->ncpus;
	series_idr->ys = system->nruns;
	series_idr->linearInterpolation = false;
	series_idr->lineType = toVector(L"dotted");
	series_idr->color = CreateRGBColor(0, 1, 1);

	ScatterPlotSeries* series_su = GetDefaultScatterPlotSeriesSettings();
	series_su->xs = system->ncpus;
	series_su->ys = system->nruns;
	series_su->linearInterpolation = false;
	series_su->lineType = toVector(L"longdash");
	series_su->color = CreateRGBColor(0, 1, 0);

	ScatterPlotSeries* series_ef = GetDefaultScatterPlotSeriesSettings();
	series_ef->xs = system->ncpus;
	series_ef->ys = system->nruns;
	series_ef->linearInterpolation = false;
	series_ef->lineType = toVector(L"dotdash");
	series_ef->color = CreateRGBColor(1, 0, 0);

    ScatterPlotSettings* settings1 = GetDefaultScatterPlotSettings();
    settings->width = 800;
    settings->height = 480;
    settings->autoBoundaries = true;
    settings->autoPadding = true;
    settings->title = toVector(L"Exec. time");
    settings->xLabel = toVector(L"Processors (int)");
    settings->yLabel = toVector(L"Runtime (ms)");
    settings->scatterPlotSeries->push_back(series_r);
    settings->scatterPlotSeries->push_back(series_idr);

	ScatterPlotSettings* settings2 = GetDefaultScatterPlotSettings();
    settings->width = 800;
    settings->height = 480;
    settings->autoBoundaries = true;
    settings->autoPadding = true;
    settings->title = toVector(L"Speedup and Efficiency");
    settings->xLabel = toVector(L"Processors (int)");
    settings->yLabel = toVector(L"Speedup (fl)");
    settings->y2Label = toVector(L"Efficiency (fl)");
    settings->scatterPlotSeries->push_back(series_su);
    settings->scatterPlotSeries->push_back(series_ef);

	DrawScatterPlotFromSettings(imageref, settings1);	
	pngData = ConvertToPNG(imageref->image);
    WriteToFile(pngData, "../graphics/plot_exec_time.png");
	DeleteImage(imageReference->image); //////////////////////////////////////////////effect???

		
	DrawScatterPlotFromSettings(imageref, settings2);
	pngData = ConvertToPNG(imageref->image);
    WriteToFile(pngData, "../graphics/plot_peedup_efficiency.png");
	DeleteImage(imageReference->image);
}
*/