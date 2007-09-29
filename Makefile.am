SUBDIRS = src

pkgdata_DATA = multi-monitor.desktop

multi-monitor.desktop: multi-monitor.desktop.in Makefile
	sed -e 's,@PKGLIBEXECDIR@,$(pkglibexecdir),g' < multi-monitor.desktop.in > multi-monitor.desktop.new
	mv multi-monitor.desktop.new multi-monitor.desktop
