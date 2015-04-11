CC=g++
CFLAGS=-I.

myfsck: main.o dbscaner.o
	$(CC) -o scaner main.o dbscaner.o -I.