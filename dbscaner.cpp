#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <vector>
#include <math.h>
#include "dbscaner.h"

#define ALLOC_NUM 1000

using namespace std;

typedef struct point_struct
{
	double x;
	double y;
	int visited;
	int isClustered;
} point;

static vector<set<point*>*> clusters;
static set<point*>* noiseCluster = new set<point*>;

static point* points;
static int p_count;

static point ppp;

void get_point(char* line, point* pnt){
	//get x
	pnt->x = atof(line);
	//get start point of y
	while(*line != ' ')
		line++;
	line++;
	//get y
	pnt->y = atof(line);
	//init as unvisited
	pnt->visited = 0;
	pnt->isClustered = 0;
}

bool isEqual(point* p1, point* p2, double diff){
	if((fabs(p1->x - p2->x) < diff) &&(fabs(p1->y - p2->y) < diff)){
		return true;
	}
	else 
		return false;
}

void load_data(char* path){
	FILE* fp;
	char line_buf[256];
	fp = fopen(path, "r");
	if(fp <= 0){
		printf("invalid input file\n");
		return;
	}
	points = (point*)malloc(ALLOC_NUM * sizeof(point));
	p_count = 0;
	while (fgets (line_buf, 256, fp)) {
    	get_point(line_buf, points + p_count);
    	p_count++;
    	//space in points is used up, need to expand it
    	if(p_count % ALLOC_NUM == 0){
    		point* tmp = points;
    		points = (point*)malloc(ALLOC_NUM * (p_count / ALLOC_NUM + 1) * sizeof(point));
    		memcpy(points, tmp, p_count * sizeof(point));
    		free(tmp);
    	}
  	}
  	fclose(fp);
}

void print_point(point* p){
	printf("[%f,%f], %d\n", p->x, p->y, p->isClustered);
}

void print_data(){
	int i = 0;
	printf("printing\n");
	for(; i < p_count; i++){
		print_point(points + i);
	}
}

void print_set(set<point*>* s){
	set<point*>::iterator it;
	for(it = s->begin(); it != s->end(); it++){
		print_point(*it);
	}
}

void print_vec(vector<point*>* vec){
	int size = vec->size();
	int i = 0;
	for(; i < size; i++){
		print_point((*vec)[i]);
	}
}

int isNeighbor(point* pnt1, point* pnt2, double eps, double diff){
	double distance = (pnt1->x - pnt2->x) * (pnt1->x - pnt2->x)
					 + (pnt1->y - pnt2->y) * (pnt1->y - pnt2->y);
	return distance < eps * eps || pnt1 == pnt2;
}

set<point*>* regionQuery(point* pnt, double eps, double diff){
	set<point*>* v = new set<point*>;
	ppp.x = 30.47791;
	ppp.y = 114.41163;
	int i;
	for(i = 0; i < p_count; i++){
		if(isEqual(&ppp, points + i, diff)){
			print_point(pnt);
		}
		if(isNeighbor(pnt, points + i, eps, diff)){
			v->insert(points + i);
			if(isEqual(&ppp, points + i, diff)){
				printf("add\n");
			}
		}
	}
	return v;
}

void join(set<point*>* s1, set<point*>* s2, vector<point*>* v2){
	set<point*>::iterator it;
	for(it = s1->begin(); it != s1->end(); it++){
		if(s2->find(*it) == s2->end()){
			s2->insert(*it);
			v2->push_back(*it);
		}
	}
}

void expandCluster(point* pnt, set<point*>* neighborPts, set<point*>* c
	, double eps, int minPts, double diff){
	printf("start expanding\n");
	ppp.x = 30.47791;
	ppp.y = 114.41163;
	c->insert(pnt);
	if(isEqual(&ppp, pnt, diff)){
		printf("changing 150\n");
	}
	pnt->isClustered = 1;
	vector<point*>* v = new vector<point*>(neighborPts->begin(), neighborPts->end());
	int i = 0;
	for(; i < v->size(); i++){
		point* p = (*v)[i];
		if(!p->visited){
			p->visited = 1;
			set<point*>* newNeighbors = regionQuery(p, eps, diff);
			if(newNeighbors->size() >= minPts){
				join(newNeighbors, neighborPts, v);
			}
			delete newNeighbors;
		}
		if(!p->isClustered){
			c->insert(p);
			p->isClustered = 1;
		}
	}
	printf("expanding done\n");
}

void cluster(char* outpath, double eps, int minPts, double diff){
	int i;
	for(i = 0; i < p_count; i++){
		if(points[i].visited)
			continue;
		points[i].visited = 1;
		set<point*>* neighborPts = regionQuery(points + i, eps, diff);
		//printf("n: %d\n", neighborPts->size());
		//mark as noise
		if(neighborPts->size() < minPts){
			noiseCluster->insert(points + i);
			//points[i].isClustered = 1;
		}
		else{
			set<point*>* c = new set<point*>;
			expandCluster(points + i, neighborPts, c, eps
							, minPts, diff);
			clusters.push_back(c);
		}
		delete neighborPts;
	}
}

void print_clusters(){
	int i = 0;
	for(; i < clusters.size(); i++){
		set<point*>* c = clusters[i];
		printf("cluster: %d\n", c->size());
		set<point*>::iterator it;
		for(it = c->begin(); it != c->end(); it++){
			print_point(*it);
		}
	}
	printf("cluster size: %d\n", clusters.size());
	printf("noise: %d\n", noiseCluster->size());
	printf("total: %d\n", clusters.size());
}
