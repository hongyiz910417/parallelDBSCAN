#include <stdio.h>
#include <stdlib.h>
#include "dbscaner.h"



int main(int argc, char **argv){
	load_data("/home/parallels/input.txt");
	//print_data();
	cluster("", 0.002, 200, 0.00000001);
	print_clusters();
}
