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

#ifndef _HAVE_CLUTTER_VLC_VIDEO_TEXTURE_H
# define _HAVE_CLUTTER_VLC_VIDEO_TEXTURE_H

# include <glib-object.h>
# include <clutter/clutter.h>

G_BEGIN_DECLS

#define CLUTTER_VLC_TYPE_VIDEO_TEXTURE clutter_vlc_video_texture_get_type()

#define CLUTTER_VLC_VIDEO_TEXTURE(obj)					\
  (G_TYPE_CHECK_INSTANCE_CAST ((obj),					\
			       CLUTTER_VLC_TYPE_VIDEO_TEXTURE, ClutterVlcVideoTexture))

#define CLUTTER_VLC_VIDEO_TEXTURE_CLASS(klass)				\
  (G_TYPE_CHECK_CLASS_CAST ((klass),					\
			    CLUTTER_VLC_TYPE_VIDEO_TEXTURE, ClutterVlcVideoTextureClass))

#define CLUTTER_VLC_IS_VIDEO_TEXTURE(obj)			\
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj),				\
			       CLUTTER_VLC_TYPE_VIDEO_TEXTURE))

#define CLUTTER_VLC_IS_VIDEO_TEXTURE_CLASS(klass)		\
  (G_TYPE_CHECK_CLASS_TYPE ((klass),				\
			    CLUTTER_VLC_TYPE_VIDEO_TEXTURE))

#define CLUTTER_VLC_VIDEO_TEXTURE_GET_CLASS(obj)			\
  (G_TYPE_INSTANCE_GET_CLASS ((obj),					\
			      CLUTTER_VLC_TYPE_VIDEO_TEXTURE, ClutterVlcVideoTextureClass))

typedef struct _ClutterVlcVideoTexture        ClutterVlcVideoTexture;
typedef struct _ClutterVlcVideoTextureClass   ClutterVlcVideoTextureClass;
typedef struct _ClutterVlcVideoTexturePrivate ClutterVlcVideoTexturePrivate;


struct _ClutterVlcVideoTexture
{
  ClutterTexture parent;

  ClutterVlcVideoTexturePrivate* priv;
};


struct _ClutterVlcVideoTextureClass
{
  ClutterTextureClass parent_class;
};


GType         clutter_vlc_video_texture_get_type(void) G_GNUC_CONST;
ClutterActor* clutter_vlc_video_texture_new(void);


G_END_DECLS

#endif /* _HAVE_CLUTTER_VLC_VIDEO_TEXTURE_H */
