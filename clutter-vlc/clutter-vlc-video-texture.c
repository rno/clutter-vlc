/*
 * Clutter-Vlc.
 *
 * VLC integration library for Clutter.
 *
 * Authored By Arnaud Vallat  <rno.rno@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "clutter-vlc-video-texture.h"
#include <vlc/vlc.h>

#include <stdlib.h>

#define TICK_TIMEOUT 0.5

struct _ClutterVlcVideoTexturePrivate
{
  libvlc_exception_t vlc_exception;
  libvlc_instance_t* vlc_instance;
  libvlc_media_player_t* vlc_media_player;

  gchar* uri;
  int buffer_percent;
  volatile gboolean is_eos;
  guint tick_timeout_id;
};

enum {
  PROP_0,

  /* ClutterMedia proprs */
  PROP_URI,
  PROP_PLAYING,
  PROP_POSITION,
  PROP_VOLUME,
  PROP_CAN_SEEK,
  PROP_BUFFER_PERCENT,
  PROP_DURATION
};


#define CLUTTER_VLC_VIDEO_TEXTURE_GET_PRIVATE(obj)			\
  (G_TYPE_INSTANCE_GET_PRIVATE((obj),					\
			       CLUTTER_VLC_TYPE_VIDEO_TEXTURE,		\
			       ClutterVlcVideoTexturePrivate))


static void clutter_vlc_media_init(ClutterMediaInterface *iface);

G_DEFINE_TYPE_WITH_CODE(ClutterVlcVideoTexture,
			clutter_vlc_video_texture,
			CLUTTER_TYPE_TEXTURE,
			G_IMPLEMENT_INTERFACE(CLUTTER_TYPE_MEDIA,
					      clutter_vlc_media_init));

/* Interface implementation */

static void
clutter_vlc_catch(libvlc_exception_t* vlc_exception)
{
  /* well this is really bad. Find out another way to handle vlc exceptions
   */

  if (libvlc_exception_raised(vlc_exception))
    {
      fprintf(stderr, "exception: %s\n", libvlc_exception_get_message(vlc_exception));
      exit(1);
    }

  libvlc_exception_clear(vlc_exception);
}


static void
clutter_vlc_event_handler(const libvlc_event_t* vlc_event, void* context)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = context;
  priv = video_texture->priv;

  switch (vlc_event->type)
    {
    case libvlc_MediaPlayerEndReached:
      priv->is_eos = TRUE;
      break;

    default:
      break;
    }
}


static gboolean
clutter_vlc_tick_timeout(gpointer data)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = data;
  priv = video_texture->priv;

  g_object_notify(G_OBJECT(video_texture), "position");

  if (priv->is_eos)
    {
      priv->is_eos = FALSE;
      g_signal_emit_by_name(G_OBJECT(video_texture), "eos");
    }

  return TRUE;
}


static const char *
clutter_vlc_get_uri(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  return priv->uri;
}


static void
clutter_vlc_set_uri(ClutterMedia* clutter_media,
		    const gchar* uri)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;
  GObject* self;
  libvlc_media_t* vlc_media;
  libvlc_event_manager_t* vlc_event_manager;
  char media_arg[96];

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;
  self = G_OBJECT(video_texture);

  if (priv->vlc_instance == NULL)
    return;

  if (priv->vlc_media_player != NULL)
    {
      libvlc_state_t state;

      state = libvlc_media_player_get_state(priv->vlc_media_player,
					    &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);

      if (state == libvlc_Opening || state == libvlc_Buffering ||
	  state == libvlc_Forward || state == libvlc_Backward ||
	  state == libvlc_Playing)
	{
	  libvlc_media_player_stop(priv->vlc_media_player, &priv->vlc_exception);
	  clutter_vlc_catch(&priv->vlc_exception);
	}

      libvlc_media_player_release(priv->vlc_media_player);
      priv->vlc_media_player = NULL;
      g_source_remove(priv->tick_timeout_id);
      priv->tick_timeout_id = 0;
    }

  if (priv->uri != NULL)
    {
      g_free(priv->uri);
      priv->uri = NULL;
    }

  priv->is_eos = FALSE;

  if (uri != NULL)
    {
      priv->uri = g_strdup(uri);
      if (priv->uri == NULL)
	return;

      vlc_media = libvlc_media_new(priv->vlc_instance,
				   priv->uri, &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);

      sprintf(media_arg, ":clutter-texture=%lld",
	      (long long int)(intptr_t)CLUTTER_TEXTURE(video_texture));

      libvlc_media_add_option(vlc_media, media_arg, &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);

      priv->vlc_media_player =
	libvlc_media_player_new_from_media(vlc_media,
					   &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);

      libvlc_media_release(vlc_media);

      vlc_event_manager =
	libvlc_media_player_event_manager(priv->vlc_media_player,
					  &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);

      libvlc_event_attach(vlc_event_manager, libvlc_MediaPlayerEndReached,
			  clutter_vlc_event_handler, video_texture,
			  &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);


      priv->tick_timeout_id = g_timeout_add(TICK_TIMEOUT * 1000,
					    clutter_vlc_tick_timeout,
					    video_texture);
    }

  g_object_notify(self, "uri");
  g_object_notify(self, "can-seek");
  g_object_notify(self, "duration");
  g_object_notify(self, "position");
}


static void
clutter_vlc_set_playing(ClutterMedia* clutter_media,
			gboolean playing)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_instance == NULL)
    return;

  if (priv->uri == NULL)
    return;

  if (priv->vlc_media_player == NULL)
    return;

  if (playing == FALSE)
    {
      libvlc_media_player_pause(priv->vlc_media_player,
				&priv->vlc_exception);
    }
  else
    {
      libvlc_media_player_play(priv->vlc_media_player,
			       &priv->vlc_exception);
    }
  clutter_vlc_catch(&priv->vlc_exception);

  g_object_notify(G_OBJECT(video_texture), "playing");
  g_object_notify(G_OBJECT(video_texture), "position");
}


static gboolean
clutter_vlc_get_playing(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;
  libvlc_state_t vlc_state;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_media_player == NULL)
    return FALSE;

  vlc_state = libvlc_media_player_get_state(priv->vlc_media_player,
					    &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  if (vlc_state != libvlc_Playing)
    return FALSE;

  return TRUE;
}


static void
clutter_vlc_set_position(ClutterMedia* clutter_media,
			 int position)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_media_player == NULL)
    return;

  libvlc_media_player_set_time(priv->vlc_media_player,
			       position * 1000,
			       &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  g_object_notify(G_OBJECT(video_texture), "position");
}


static int
clutter_vlc_get_position(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;
  libvlc_time_t ret;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_media_player == NULL)
    return 0.0;

  ret = libvlc_media_player_get_time(priv->vlc_media_player,
					  &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  return (int)(ret / 1000);
}


static void
clutter_vlc_set_volume(ClutterMedia* clutter_media,
		       gdouble volume)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_instance == NULL)
    return;

  libvlc_audio_set_volume(priv->vlc_instance,
			  (int)(volume * 2.0),
			  &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  g_object_notify(G_OBJECT(video_texture), "volume");
}


static gdouble
clutter_vlc_get_volume(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;
  gdouble volume;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_instance == NULL)
    return 0.0;

  volume = (gdouble)libvlc_audio_get_volume(priv->vlc_instance,
					    &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  return CLAMP(volume / 2.0, 0.0, 100.0);
}


static gboolean
clutter_vlc_can_seek(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;
  int ret;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_media_player == NULL)
    return FALSE;

  ret = libvlc_media_player_is_seekable(priv->vlc_media_player,
					&priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  if (ret == 0)
    return FALSE;

  return TRUE;
}


static int
clutter_vlc_get_duration(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;
  libvlc_time_t ret;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  if (priv->vlc_media_player == NULL)
    return 0;

  ret = libvlc_media_player_get_length(priv->vlc_media_player,
				       &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  return (int)ret / 1000;
}


static int
clutter_vlc_get_buffer_percent(ClutterMedia* clutter_media)
{
  ClutterVlcVideoTexture* video_texture;
  ClutterVlcVideoTexturePrivate* priv;

  video_texture = CLUTTER_VLC_VIDEO_TEXTURE(clutter_media);
  priv = video_texture->priv;

  return priv->buffer_percent;
}


static void
clutter_vlc_media_init(ClutterMediaInterface* iface)
{
  iface->set_uri = clutter_vlc_set_uri;
  iface->get_uri = clutter_vlc_get_uri;
  iface->set_playing = clutter_vlc_set_playing;
  iface->get_playing = clutter_vlc_get_playing;
  iface->set_position = clutter_vlc_set_position;
  iface->get_position = clutter_vlc_get_position;
  iface->set_volume = clutter_vlc_set_volume;
  iface->get_volume = clutter_vlc_get_volume;
  iface->can_seek = clutter_vlc_can_seek;
  iface->get_buffer_percent = clutter_vlc_get_buffer_percent;
  iface->get_duration = clutter_vlc_get_duration;
}

static void
clutter_vlc_video_texture_dispose(GObject* object)
{
  ClutterVlcVideoTexture* self;
  ClutterVlcVideoTexturePrivate* priv;

  self = CLUTTER_VLC_VIDEO_TEXTURE(object);
  priv = self->priv;

  if (priv->vlc_media_player != NULL)
    {
      libvlc_media_player_stop(priv->vlc_media_player,
			       &priv->vlc_exception);
      clutter_vlc_catch(&priv->vlc_exception);

      libvlc_media_player_release(priv->vlc_media_player);
      priv->vlc_media_player = NULL;

      g_source_remove(priv->tick_timeout_id);
      priv->tick_timeout_id = 0;
    }

  if (priv->vlc_instance != NULL)
    {
      libvlc_release(priv->vlc_instance);
      priv->vlc_instance = NULL;
    }

  G_OBJECT_CLASS(clutter_vlc_video_texture_parent_class)->dispose(object);
}


static void
clutter_vlc_video_texture_finalize(GObject* object)
{
  ClutterVlcVideoTexture* self;
  ClutterVlcVideoTexturePrivate* priv;

  self = CLUTTER_VLC_VIDEO_TEXTURE(object);
  priv = self->priv;

  if (priv->uri != NULL)
    {
      g_free(priv->uri);
      priv->uri = NULL;
    }

  G_OBJECT_CLASS(clutter_vlc_video_texture_parent_class)->finalize(object);
}


static void
clutter_vlc_video_texture_set_property(GObject* object,
				       guint property_id,
				       const GValue* value,
				       GParamSpec* pspec)
{
  switch (property_id)
    {
    case PROP_URI:
      clutter_vlc_set_uri(CLUTTER_MEDIA(object), g_value_get_string(value));
      break;

    case PROP_PLAYING:
      clutter_vlc_set_playing(CLUTTER_MEDIA(object), g_value_get_boolean(value));
      break;

    case PROP_POSITION:
      clutter_vlc_set_position(CLUTTER_MEDIA(object), g_value_get_int(value));
      break;

    case PROP_VOLUME:
      clutter_vlc_set_volume(CLUTTER_MEDIA(object), g_value_get_double(value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}


static void
clutter_vlc_video_texture_get_property(GObject* object,
				       guint property_id,
				       GValue* value,
				       GParamSpec* pspec)
{
  switch (property_id)
    {
    case PROP_URI:
      g_value_set_string(value, clutter_vlc_get_uri(CLUTTER_MEDIA(object)));
      break;

    case PROP_PLAYING:
      g_value_set_boolean(value, clutter_vlc_get_playing(CLUTTER_MEDIA(object)));
      break;

    case PROP_POSITION:
      g_value_set_int(value, clutter_vlc_get_position(CLUTTER_MEDIA(object)));
      break;

    case PROP_VOLUME:
      g_value_set_double(value, clutter_vlc_get_volume(CLUTTER_MEDIA(object)));
      break;

    case PROP_CAN_SEEK:
      g_value_set_boolean(value, clutter_vlc_can_seek(CLUTTER_MEDIA(object)));
      break;

    case PROP_BUFFER_PERCENT:
      g_value_set_int(value, clutter_vlc_get_buffer_percent(CLUTTER_MEDIA(object)));
      break;

    case PROP_DURATION:
      g_value_set_int(value, clutter_vlc_get_duration(CLUTTER_MEDIA(object)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}


static void
clutter_vlc_video_texture_class_init(ClutterVlcVideoTextureClass* klass)
{
  GObjectClass* object_class;

  object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = clutter_vlc_video_texture_dispose;
  object_class->finalize = clutter_vlc_video_texture_finalize;
  object_class->set_property = clutter_vlc_video_texture_set_property;
  object_class->get_property = clutter_vlc_video_texture_get_property;

  /* Interface props */
  g_object_class_override_property(object_class, PROP_URI, "uri");
  g_object_class_override_property(object_class, PROP_PLAYING, "playing");
  g_object_class_override_property(object_class, PROP_POSITION, "position");
  g_object_class_override_property(object_class, PROP_VOLUME, "volume");
  g_object_class_override_property(object_class, PROP_CAN_SEEK, "can-seek");
  g_object_class_override_property(object_class, PROP_DURATION, "duration");
  g_object_class_override_property(object_class, PROP_BUFFER_PERCENT, "buffer-percent" );

  g_type_class_add_private(klass, sizeof(ClutterVlcVideoTexturePrivate));
}


static void
clutter_vlc_video_texture_init(ClutterVlcVideoTexture* video_texture)
{
  ClutterVlcVideoTexturePrivate* priv;
  char const * vlc_argv[] =
    {
      "--ignore-config",
      "--vout", "clutter",
    };
  int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

  video_texture->priv = CLUTTER_VLC_VIDEO_TEXTURE_GET_PRIVATE(video_texture);
  priv = video_texture->priv;

  libvlc_exception_init(&priv->vlc_exception);
  priv->vlc_instance = libvlc_new(vlc_argc, vlc_argv, &priv->vlc_exception);
  clutter_vlc_catch(&priv->vlc_exception);

  priv->vlc_media_player = NULL;
  priv->uri = NULL;
}


ClutterActor *
clutter_vlc_video_texture_new(void)
{
  return g_object_new(CLUTTER_VLC_TYPE_VIDEO_TEXTURE,
		      NULL);
}
