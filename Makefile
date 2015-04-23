CC=g++

CFLAGS      = -O3 -I.
OMPFLAGS    = -fopenmp -lpthread

#H_FILES     = dbscaner.h
%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(OMPFLAGS) -std=c++11
scaner: main.o dbscaner.o
	$(CC) $(CFLAGS) $(OMPFLAGS) -o scaner main.o dbscaner.o -I.
#g++ -c main.cpp -o main.o -fopenmp -O3
#g++ main.o -o scaner -fopenmp -lpthread -O3
#g++ main.cpp -fopenmp -lpthread 

#dbscaner.o: dbscaner.cpp $(H_FILES)
#		$(CC) -lm -msse3 -O3 $(CFLAGS) $(OMPFLAGS) -c dbscaner.cpp
#main.o: main.cpp
#		$(CC) -O3 $(CFLAGS) -c main.cpp
#scaner: dbscaner.o main.o
#		$(CC) -lm -msse3 -O3 -o scaner dbscaner.o main.o -I.