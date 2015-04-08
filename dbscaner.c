#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbscaner.h"

#define ALLOC_NUM 1000

typedef struct point_struct
{
	double x;
	double y;
} point;

static point* points;
static int p_count;

void get_point(char* line, point* pnt){
	//get x
	pnt->x = atof(line);
	//get start point of y
	while(*line != ',')
		line++;
	line++;
	//get y
	pnt->y = atof(line);
}

void load_data(char* path){
	FILE* fp;
	char line_buf[256];
	fp = fopen(path, "r");
	if(fp == NULL){
		printf("invalid input file\n");
		return;
	}
	points = malloc(ALLOC_NUM * sizeof(point));
	p_count = 0;
	while (fgets (line_buf, 256, fp)) {
    	get_point(line_buf, points + p_count);
    	p_count++;
    	//space in points is used up, need to expand it
    	if(p_count % ALLOC_NUM == 0){
    		point* tmp = points;
    		points = malloc(ALLOC_NUM * (p_count / ALLOC_NUM + 1) * sizeof(point));
    		memcpy(points, tmp, p_count * sizeof(point));
    		free(tmp);
    	}
  	}
}

void print_data(){
	int i = 0;
	for(; i < p_count; i++){
		printf("%f,%f\n", points[i].x, points[i].y);
	}
}

void cluster(char* outpath){

}