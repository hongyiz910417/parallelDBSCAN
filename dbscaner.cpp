#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <vector>
#include <math.h>
#include <__debug>
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

/*typedef struct node_struct{
	point* val;
	node* next;
} node;

typedef struct cluster_struct{
	int count;
	cluster* next_cluster;
	node* next_node;
} cluster;

typedef struct cluster_header_struct{
	int count;
	cluster* next_cluster;
} cluster_header;*/

static vector<set<point*>*> clusters;
static set<point*>* noiseCluster = new set<point*>;

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
	//init as unvisited
	pnt->visited = 0;
	pnt->isClustered = 0;
}

void load_data(char* path){
	FILE* fp;
	char line_buf[256];
	fp = fopen(path, "r");
	if(fp == NULL){
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
}

void print_point(point* p){
	printf("%f,%f,%d,%d\n", p->x, p->y, p->visited
				, p->isClustered);
}

void print_data(){
	int i = 0;
	for(; i < p_count; i++){
		print_point(points + i);
	}
}

/*void free_cluster(cluster* list){
	node* p = list->next_node;
	while(p != NULL){
		node* tmp = p;
		p = p->next;
		free(tmp);
	}
	free(list);
}

void add_node(cluster* list, point* p){
	node* n = malloc(sizeof(node));
	n->val = p;
	list->count++;
	n->next = list->next_node;
	list->next_node = n;
}

void add_cluster(cluster_header* list, cluster* c){
	list->count++;
	c->next_cluster = list->next_cluster;
	list->next_cluster = c;
}*/

void print_set(set<point*>* s){
	set<point*>::iterator it;
	for(it = s->begin(); it != s->end(); it++){
		print_point(*it);
	}
}

int isNeighbor(point* pnt1, point* pnt2, double eps, double diff){
	double distance = (pnt1->x - pnt2->x) * (pnt1->x - pnt2->x)
					 + (pnt1->y - pnt2->y) * (pnt1->y - pnt2->y);
	//printf("dis: %f\n", fabs(distance - eps * eps));
	return distance < eps * eps || pnt1 == pnt2;
}

set<point*>* regionQuery(point* pnt, double eps, double diff){
	set<point*>* v = new set<point*>;
	int i;
	for(i = 0; i < p_count; i++){
		if(isNeighbor(pnt, points + i, eps, diff)){
			v->insert(points + i);
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
	c->insert(pnt);
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
}

void cluster(char* outpath, double eps, int minPts, double diff){
	/*cluster_header* clusters = malloc(sizeof(cluster_header));
	clusters->count = 0;
	clusters->next_cluster = NULL;
	//init noise points cluster
	cluster* noiseCluster = malloc(sizeof(cluster));
	noiseCluster->count = 0;
	noiseCluster->next_cluster = NULL;
	noiseCluster->next_node = NULL;*/

	int i;
	for(i = 0; i < p_count; i++){
		if(points[i].visited)
			continue;
		points[i].visited = 1;
		set<point*>* neighborPts = regionQuery(points + i, eps, diff);
		printf("n:\n");
		print_set(neighborPts);
		//mark as noise
		if(neighborPts->size() < minPts){
			noiseCluster->insert(points + i);
			points[i].isClustered = 1;
		}
		else{
			set<point*>* c = new set<point*>;
			expandCluster(points + i, neighborPts, c, eps
							, minPts, diff);
			//add_cluster(clusters, c);
			clusters.push_back(c);
		}
		delete neighborPts;
	}
}

void print_clusters(){
	int i = 0;
	for(; i < clusters.size(); i++){
		set<point*>* c = clusters[i];
		printf("cluster:\n");
		set<point*>::iterator it;
		for(it = c->begin(); it != c->end(); it++){
			print_point(*it);
		}
	}
}