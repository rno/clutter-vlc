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

#ifndef __CLUTTER_VLC_VERSION_H__
# define __CLUTTER_VLC_VERSION_H__

# define CLUTTER_VLC_MAJOR_VERSION   (0)

# define CLUTTER_VLC_MINOR_VERSION   (9)

# define CLUTTER_VLC_MICRO_VERSION   (0)

# define CLUTTER_VLC_VERSION         (0.9.0)

# define CLUTTER_VLC_VERSION_S       "0.9.0"

# define CLUTTER_VLC_VERSION_HEX     ((CLUTTER_VLC_MAJOR_VERSION << 24) | \
				      (CLUTTER_VLC_MINOR_VERSION << 16) | \
				      (CLUTTER_VLC_MICRO_VERSION << 8))

# define CLUTTER_VLC_CHECK_VERSION(major,minor,micro)			\
  (pCLUTTER_VLC_MAJOR_VERSION > (major) ||				\
   (CLUTTER_VLC_MAJOR_VERSION == (major) && CLUTTER_VLC_MINOR_VERSION > (minor)) || \
   (CLUTTER_VLC_MAJOR_VERSION == (major) && CLUTTER_VLC_MINOR_VERSION == (minor) && CLUTTER_VLC_MICRO_VERSION >= (micro)))

#endif /* __CLUTTER_VLC_VERSION_H__ */
