all: test

test: test.c mccgraph.c mccgraph.h mccvalue.c mccvalue.h linux_cpuload.c
	gcc -Wall -o test `pkg-config --cflags pangoft2 gtk+-2.0 glib-2.0` test.c mccgraph.c mccvalue.c linux_cpuload.c `pkg-config --libs pangoft2 gtk+-2.0 glib-2.0`
