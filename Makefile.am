SUBDIRS = clutter-vlc examples

DIST_SUBDIRS = clutter-vlc examples

clutter-vlc-0.8.pc: clutter-vlc.pc
	@cp -f clutter-vlc.pc clutter-vlc-0.8.pc

pkgconfig_DATA = clutter-vlc-0.8.pc
pkgconfigdir   = $(libdir)/pkgconfig

EXTRA_DIST = clutter-vlc.pc.in

CLEANFILES = clutter-vlc-0.8.pc

DISTCLEANFILES = clutter-vlc.pc

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub \
                       configure depcomp install-sh ltmain.sh     \
                       Makefile.in missing config.h.in


