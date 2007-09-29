pkglibexec_PROGRAMS = multi-monitor

pkglibexecdir = $(libexecdir)/xfce4

multi_monitor_SOURCES = \
    main.c		\
    main.h		\
    preferences.c	\
    preferences.h	\
    mccgraph.c		\
    mccgraph.h		\
    mccvalue.c		\
    mccvalue.h		\
    mccdatasource.c	\
    mccdatasource.h	\
    mccsrcbattery.c	\
    mccsrcbattery.h	\
    mccsrccpufreq.c	\
    mccsrccpufreq.h	\
    mccsrccpuload.c	\
    mccsrccpuload.h	\
    mccsrcdiskio.c	\
    mccsrcdiskio.h	\
    mccsrcloadavg.c	\
    mccsrcloadavg.h	\
    mccsrcmemory.c	\
    mccsrcmemory.h

AM_CPPFLAGS = $(GTK_CFLAGS)
# -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = -Wall -fgnu89-inline
LDADD = $(GTK_LIBS)
