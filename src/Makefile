all: test

SRCS = test.c datasrc.h mccgraph.c mccgraph.h mccvalue.c mccvalue.h linux_cpuload.c linux_cpufreq.c

test: $(SRCS)
	rm -f test
	gcc -g -Wall -o test `pkg-config --cflags pangoft2 gtk+-2.0 glib-2.0` $(SRCS) `pkg-config --libs pangoft2 gtk+-2.0 glib-2.0`
