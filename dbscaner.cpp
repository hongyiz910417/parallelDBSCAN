#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <vector>
#include <math.h>
#include <omp.h>
#include "dbscaner.h"

#define P_WIDTH 4
#define P_HEIGHT 4
#define THREAD_NUM 16

using namespace std;

typedef struct point_struct
{
	double x;
	double y;
	int visited;
	int isClustered;
	int type; //0: null, 1: core, 2: border
	int p_x;
	int p_y;
	set<point_struct*>* belongs_to;
} point;

static set<set<point*>*> clusters;
static set<point*>* noiseCluster = new set<point*>;
static vector<vector<point*>* > partitions;
static vector<vector<pair<point*, point*> >*> p_records;
static vector<set<set<point*>*>*> p_clusters;

static vector<point*> points;
static int p_count;

void print_point(point* p){
	printf("[%f,%f], %d\n", p->x, p->y, p->isClustered);
}

void print_data(){
	int i = 0;
	printf("printing\n");
	for(; i < p_count; i++){
		print_point(points[i]);
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

void check_zero(){
	int i;
	for(i = 0; i < p_count; i++){
		if(fabs(points[i]->x - 0) < 0.0000001)
			printf("aha\n");
	}
}

void check_zero(vector<point*>* v){
	int i = 0;
	for(;i < v->size(); i++){
		if(fabs((*v)[i]->x - 0) < 0.000000001){
			printf("aha, %d\n", i);
			print_point((*v)[i]);
			exit(1);
		}
	}
}

int get_p_x(double x){
	int keyx;
	if (x < 30.519882857142857) {
		keyx = 0;
	} 
	else if (x < 30.565401904761902) {
		keyx = 1;
	} 
	else if (x < 30.612134793650792) {
		keyx = 2;
	} 
	else {
		keyx = 3;
	}
	return keyx;
}

int get_p_y(double y){
	int keyy;
	if (y < 114.24973692307692) {
		keyy = 0;
	} 
	else if (y < 114.32396532051283) {
		keyy = 1;
	} 
	else if (y < 114.37465788461539) {
		keyy = 2;
	} 
	else {
		keyy = 3;
	}
	return keyy;
}

bool isEqual(point* p1, point* p2, double diff){
	if((fabs(p1->x - p2->x) < diff) &&(fabs(p1->y - p2->y) < diff)){
		return true;
	}
	else 
		return false;
}

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
	pnt->type = 0;
	pnt->belongs_to = NULL;
	pnt->p_x = get_p_x(pnt->x);
	pnt->p_y = get_p_y(pnt->y);
	partitions[pnt->p_y * P_WIDTH + pnt->p_x]->push_back(pnt);
}

void load_data(char* path){
	FILE* fp;
	char line_buf[256];
	fp = fopen(path, "r");
	if(fp <= 0){
		printf("invalid input file\n");
		return;
	}

	int i;
	for(i = 0; i < P_HEIGHT * P_WIDTH; i++){
		partitions.push_back(new vector<point*>);
		p_records.push_back(new vector<pair<point*, point*> >);
		p_clusters.push_back(new set<set<point*>*>);
	}
	//printf("len: %d\n", partitions[15]->size());

	//points = (point*)malloc(ALLOC_NUM * sizeof(point));
	p_count = 0;
	while (fgets (line_buf, 256, fp)) {
		char* cp = line_buf;
		while(*cp != ',')
			cp++;
		cp++;
		point* pnt = (point*)malloc(sizeof(point));
    	get_point(cp, pnt);
    	p_count++;
    	points.push_back(pnt);
    	// space in points is used up, need to expand it
    	// if(p_count % ALLOC_NUM == 0){
    	// 	point* tmp = points;
    	// 	points = (point*)malloc(ALLOC_NUM * (p_count / ALLOC_NUM + 1) * sizeof(point));
    	// 	memcpy(points, tmp, p_count * sizeof(point));
    	// 	free(tmp);
    	// }
  	}
  	//check_zero(partitions[15]);
  	//printf("out of here\n");
  	fclose(fp);
}

int isNeighbor(point* pnt1, point* pnt2, double eps, double diff){
	double distance = (pnt1->x - pnt2->x) * (pnt1->x - pnt2->x)
					 + (pnt1->y - pnt2->y) * (pnt1->y - pnt2->y);
	return distance < eps * eps || pnt1 == pnt2;
}

set<point*>* regionQuery(point* pnt, double eps, double diff){
	set<point*>* v = new set<point*>;
	int i;
	for(i = 0; i < p_count; i++){
		if(isNeighbor(pnt, points[i], eps, diff)){
			v->insert(points[i]);
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

void expandCluster(int partition_id, point* pnt, set<point*>* neighborPts, set<point*>* c
	, double eps, int minPts, double diff){
	//check_zero(partitions[partition_id]);
	c->insert(pnt);
	pnt->belongs_to = c;
	pnt->isClustered = 1;
	pnt->type = 1;
	vector<point*>* v = new vector<point*>(neighborPts->begin(), neighborPts->end());
	int i = 0;
	//check_zero(partitions[partition_id]);
	for(; i < v->size(); i++){
		int type_flag = 2;
		point* p = (*v)[i];
		if(p->p_y * P_WIDTH + p->p_x != partition_id){
			//printf("%d partition_id, %d actual\n", partition_id, p->p_y * P_WIDTH + p->p_x);
			p_records[partition_id]->push_back(make_pair(pnt, p));
			continue;
		}
		if(!p->visited){
			p->visited = 1;
			set<point*>* newNeighbors = regionQuery(p, eps, diff);
			if(newNeighbors->size() >= minPts){
				join(newNeighbors, neighborPts, v);
				type_flag = 1;
			}
			delete newNeighbors;
		}
		if(!p->isClustered){
			c->insert(p);
			p->isClustered = 1;
			p->type = type_flag;
			p->belongs_to = c;
		}
	}
}

void cluster_partition(int partition_id, double eps, int minPts, double diff){
	int i;
	printf("into partition: %d\n", partition_id);
	vector<point*>* partition = partitions[partition_id];
	for(i = 0; i < partition->size(); i++){
		if(((*partition)[i])->visited)
			continue;
		((*partition)[i])->visited = 1;
		set<point*>* neighborPts = regionQuery((*partition)[i], eps, diff);
		//mark as noise
		if(neighborPts->size() < minPts){
			noiseCluster->insert((*partition)[i]);
		}
		else{
			//printf("into here\n");
			set<point*>* c = new set<point*>;
			expandCluster(partition_id, (*partition)[i], neighborPts, c, eps
							, minPts, diff);
			p_clusters[partition_id]->insert(c);
			//check_zero(partition);
		}
		delete neighborPts;
		//check_zero(partitions[15]);
	}
	//printf("partition %d: %d\n", partition_id, partition->size());
}

void merge_clusters(point* p1, point* p2){
	set<point*>* c1 = p1->belongs_to;
	set<point*>* c2 = p2->belongs_to;
	if(c1 == c2)
		return;
	set<point*>::iterator it;
	for(it = c2->begin(); it != c2->end(); it++){
		c1->insert(*it);
		(*it)->belongs_to = c1;
	}
	int par_id2 = p2->p_y * P_WIDTH + p2->p_x;
	p_clusters[par_id2]->erase(c2);
	p_clusters[par_id2]->insert(c1);
	delete c2;
}

void merge(int partition_id){
	vector<pair<point*, point*> >* pairs = p_records[partition_id];
	int i = 0;
	for(i = 0; i < pairs->size(); i++){
		if((*pairs)[i].second->type == 1){
			merge_clusters((*pairs)[i].first, (*pairs)[i].second);
		}
		else if((*pairs)[i].second->isClustered == 0){
			(*pairs)[i].first->belongs_to->insert((*pairs)[i].second);
			(*pairs)[i].second->isClustered = 1;
			(*pairs)[i].second->belongs_to = (*pairs)[i].first->belongs_to;
			(*pairs)[i].second->type = 2;
		}
	}
}

void cluster(char* outpath, double eps, int minPts, double diff){
	//for(i = 0; i < P_HEIGHT * P_WIDTH; i++){
	int i;
	#pragma omp parallel for num_threads(THREAD_NUM) private(i)
	for(i = 0; i < 16; i++){	
		printf("thread: %d\n", omp_get_thread_num());
		cluster_partition(i, eps, minPts, diff);
	}

	for(i = 0; i < P_WIDTH * P_HEIGHT; i++){
		printf("%d part: %d\n", partitions[i]->size());
	}

	//#pragma omp parallel num_threads(THREAD_NUM){
	for(i = 0; i < P_HEIGHT * P_WIDTH; i++){
		merge(i);
	}
	for(i = 0; i < P_HEIGHT * P_WIDTH; i++){
		set<set<point*>*>* s = p_clusters[i];
		set<set<point*>*>::iterator it;
		for(it = s->begin(); it != s->end(); it++){
			clusters.insert(*it);
		}
	}
}

void print_clusters(){
	int i = 0;
	set<set<point*>*>::iterator iter;
	for(iter = clusters.begin(); iter != clusters.end(); iter++){
		printf("i: %d\n", i);
		i++;
		set<point*>* c = *iter;
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
