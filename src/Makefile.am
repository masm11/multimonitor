pkglibexec_PROGRAMS = multi-monitor

multi_monitor_SOURCES = \
    battery.c		\
    main.c		\
    main.h		\
    about.c		\
    about.h		\
    sysfs.c		\
    sysfs.h		\
    cpufreq.c		\
    cpufreq.h		\
    loadavg.c		\
    loadavg.h		\
    cpuload.c		\
    cpuload.h		\
    net.c		\
    net.h		\
    list.c		\
    list.h		\
    line.c		\
    line.h

#    preferences.c	\
#    preferences.h	\
#    mccgraph.c		\
#    mccgraph.h		\
#    mccvalue.c		\
#    mccvalue.h		\
#    mccdatasource.c	\
#    mccdatasource.h	\
#    mccsrcbattery.c	\
#    mccsrcbattery.h	\
#    mccsrccpufreq.c	\
#    mccsrccpufreq.h	\
#    mccsrccpuload.c	\
#    mccsrccpuload.h	\
#    mccsrcdiskio.c	\
#    mccsrcdiskio.h	\
#    mccsrcloadavg.c	\
#    mccsrcloadavg.h	\
#    mccsrcmemory.c	\
#    mccsrcmemory.h	\
#    battalert.c		\
#    battalert.h		\
#    opendirat.c		\
#    opendirat.h

AM_CPPFLAGS = $(GTK_CFLAGS)
# -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = -Wall -fgnu89-inline
LDADD = $(GTK_LIBS) -lm
