source_h = 					\
	$(srcdir)/clutter-vlc-video-texture.h 	\
	$(srcdir)/clutter-vlc-main.h 		\
	$(srcdir)/clutter-vlc-version.h		\
	$(srcdir)/clutter-vlc.h


source_c = clutter-vlc-video-texture.c \
	   clutter-vlc-main.c

libclutter_vlc_@CLUTTER_VLC_MAJORMINOR@_la_SOURCES = $(MARSHALFILES)  \
                                                     $(source_c)      \
                                                     $(source_h)

INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"Clutter-Vlc\" \
	@GCC_FLAGS@ \
	@CLUTTER_VLC_CFLAGS@ \
	$(VLC_CFLAGS)

lib_LTLIBRARIES = libclutter-vlc-@CLUTTER_VLC_MAJORMINOR@.la

libclutter_vlc_@CLUTTER_VLC_MAJORMINOR@_la_LIBADD  = @CLUTTER_VLC_LIBS@ $(VLC_LIBS)
libclutter_vlc_@CLUTTER_VLC_MAJORMINOR@_la_LDFLAGS = @CLUTTER_VLC_LT_LDFLAGS@

cluttervlcheadersdir = $(includedir)/clutter-0.9/clutter-vlc
cluttervlcheaders_HEADERS = $(source_h)
