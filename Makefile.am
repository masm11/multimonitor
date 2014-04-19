SUBDIRS = src

EXTRA_DIST = multi-monitor.desktop.in

CLEANFILES = $(pkgdata_DATA)

pkgdata_DATA = multi-monitor.desktop

multi-monitor.desktop: $(srcdir)/multi-monitor.desktop.in Makefile
	sed -e 's,@PKGLIBEXECDIR@,$(pkglibexecdir),g' < $(srcdir)/multi-monitor.desktop.in > multi-monitor.desktop.new
	mv multi-monitor.desktop.new multi-monitor.desktop
