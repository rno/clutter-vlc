noinst_PROGRAMS = video-player

INCLUDES = -I$(top_srcdir)/

video_player_SOURCES = video-player.c
video_player_CFLAGS = $(CLUTTER_VLC_CFLAGS) $(VLC_CFLAGS)
video_player_LDFLAGS = \
    $(CLUTTER_VLC_LIBS) \
    $(VLC_LIBS)   \
    $(top_builddir)/clutter-vlc/libclutter-vlc-@CLUTTER_VLC_MAJORMINOR@.la
