all: test

PKGS = libxfce4panel-1.0 pangoft2 gtk+-2.0 glib-2.0

SRCS = main.c mccgraph.c mccgraph.h mccvalue.c mccvalue.h preferences.c mccdatasource.c mccsrccpufreq.c mccsrccpuload.c mccsrcbattery.c mccsrcloadavg.c mccsrcmemory.c

test: $(SRCS)
	rm -f test
	gcc -g -Wall -std=gnu99 -fgnu89-inline -o test `pkg-config --cflags $(PKGS)` $(SRCS) `pkg-config --libs $(PKGS)`
