all: test

test: test.c mccgraph.c mccgraph.h
	gcc -o test `pkg-config --cflags pangoft2 gtk+-2.0` test.c mccgraph.c `pkg-config --libs pangoft2 gtk+-2.0`
