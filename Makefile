all: bame

bame: bame.c bame.h
	gcc -g -o bame bame.c -lcurses -lpthread
