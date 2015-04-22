#include <stdio.h>
#include <stdlib.h>
#include "dbscaner.h"

int main(int argc, char **argv){
	load_data("/home/parallels/output");
	//print_data();
	cluster("", 0.001, 20, 0.00000001);
	print_clusters();
}
