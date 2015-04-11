#include "dbscaner.h"

int main(int argc, char **argv){
	load_data("/Users/d/Desktop/test");
	print_data();
	cluster("", 10.0, 2, 0.000000001);
	print_clusters();
}