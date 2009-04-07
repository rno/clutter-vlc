# clutter package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress developmen
# An even micro number indicates a released version.
m4_define(clutter_vlc_major_version, 0)
m4_define(clutter_vlc_minor_version, 8)
m4_define(clutter_vlc_micro_version, 0)

m4_define([clutter_vlc_version],
          [clutter_vlc_major_version.clutter_vlc_minor_version.clutter_vlc_micro_version])

m4_define([clutter_vlc_api_version],
          [clutter_vlc_major_version.clutter_vlc_minor_version])

# increase the interface age for each release; if the API changes, set to 0
m4_define([clutter_vlc_interface_age], [0])
m4_define([clutter_vlc_binary_age],
          [m4_eval(100 * clutter_vlc_minor_version + clutter_vlc_micro_version)])

AC_PREREQ(2.53)

AC_INIT([clutter-vlc], [clutter_vlc_version],
        [])
AC_CONFIG_SRCDIR([clutter-vlc/clutter-vlc.h])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.9])

CLUTTER_VLC_MAJOR_VERSION=clutter_vlc_major_version
CLUTTER_VLC_MINOR_VERSION=clutter_vlc_minor_version
CLUTTER_VLC_MICRO_VERSION=clutter_vlc_micro_version
CLUTTER_VLC_VERSION=clutter_vlc_version
CLUTTER_VLC_API_VERSION=clutter_vlc_api_version
CLUTTER_VLC_MAJORMINOR=clutter_vlc_api_version
AC_SUBST(CLUTTER_VLC_MAJOR_VERSION)
AC_SUBST(CLUTTER_VLC_MINOR_VERSION)
AC_SUBST(CLUTTER_VLC_MICRO_VERSION)
AC_SUBST(CLUTTER_VLC_VERSION)
AC_SUBST(CLUTTER_VLC_API_VERSION)
AC_SUBST(CLUTTER_VLC_MAJORMINOR)

m4_define([lt_current],
          [m4_eval(100 * clutter_vlc_minor_version + clutter_vlc_micro_version - clutter_vlc_interface_age)])
m4_define([lt_revision], [clutter_vlc_interface_age])
m4_define([lt_age], [m4_eval(clutter_vlc_binary_age - clutter_vlc_interface_age)])

CLUTTER_VLC_LT_CURRENT=lt_current
CLUTTER_VLC_LT_REV=lt_revision
CLUTTER_VLC_LT_AGE=lt_age
CLUTTER_VLC_LT_VERSION="$CLUTTER_VLC_LT_CURRENT:$CLUTTER_VLC_LT_REV:$CLUTTER_VLC_LT_AGE"
CLUTTER_VLC_LT_LDFLAGS="-version-info $CLUTTER_VLC_LT_VERSION"

AC_SUBST(CLUTTER_VLC_LT_VERSION)
AC_SUBST(CLUTTER_VLC_LT_LDFLAGS)

dnl ========================================================================

# Checks for programs.
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strcasecmp strdup])


dnl ========================================================================

pkg_modules="clutter-0.8 >= 0.8.0"
PKG_CHECK_MODULES(CLUTTER_VLC, [$pkg_modules])

dnl ========================================================================

pkg_modules="libvlc >= 0.9.8"
PKG_CHECK_MODULES(VLC, [$pkg_modules])

dnl ========================================================================

if test "x$GCC" = "xyes"; then
        GCC_FLAGS="-g -Wall"
fi

dnl ========================================================================

AC_SUBST(GCC_FLAGS)
AC_SUBST(VLC_CFLAGS)
AC_SUBST(VLC_LIBS)

AC_SUBST(CLUTTER_VLC_CFLAGS)
AC_SUBST(CLUTTER_VLC_LIBS)

AC_OUTPUT([
        Makefile
        clutter-vlc/Makefile
        clutter-vlc/clutter-vlc-version.h
        examples/Makefile
        clutter-vlc.pc
])

dnl ========================================================================

echo ""
echo "                  Clutter-VLC $VERSION"
echo "                 ==================="
echo ""
echo "                   prefix:   ${prefix}"
echo ""
