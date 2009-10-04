dnl ==============================================================
dnl Process this file with autoconf to produce a configure script.
dnl ==============================================================

AC_INIT([MuSE],[0.10])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/muse.cpp])

dnl ==============================================================
dnl Setup for automake
dnl ==============================================================

AM_INIT_AUTOMAKE([dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# never run autotools and configure automatically
AM_MAINTAINER_MODE

dnl ==============================================================
dnl Get the operating system and version number...
dnl ==============================================================
AC_MSG_CHECKING(for which platform we are compiling)
case "$host_os" in
  *linux*)
    AC_MSG_RESULT([Linux])
    # HAVE_LINUX/UNIX are needed by portaudio only
    AC_DEFINE(HAVE_LINUX,1,[define if compiling for Linux])
    AC_DEFINE(HAVE_UNIX,1,[define if compiling for UNIX])
    have_linux=yes
  ;;

  *darwin*)
    AC_MSG_RESULT([Darwin/OSX])
    AC_DEFINE(HAVE_DARWIN,1,[define if compiling for Apple Darwin OSX])
    DARWIN_LIBS="-L/sw/lib -lintl -framework AudioToolbox -framework CoreAudio"
    DARWIN_CFLAGS="-I/sw/include"
    have_darwin=yes
  ;;

  *bsd*)
    AC_MSG_RESULT(${host})
    AC_DEFINE(HAVE_BSD,1,[define if compiling for BSD])
    AC_DEFINE(HAVE_UNIX,1,[define if compiling for UNIX])
    have_bsd=yes
  ;;

  *)
    AC_MSG_RESULT([$host_os?!])
    AC_MSG_ERROR([[
[!] Your system architecture is not supported by MuSE
[!] if you are interested in porting MuSE to your architecture
[!] you are very welcome to contact me <jaromil@dyne.org>
]], 0)
  ;;
esac


#GLOBAL_CFLAGS="-O4 -D_REENTRANT -fomit-frame-pointer -march=${host_cpu} -ffast-math -malign-double -funroll-loops -pipe"
GLOBAL_CFLAGS="-pipe -D_REENTRANT -Wall"


dnl ==============================================================
dnl Check for tools
dnl ==============================================================
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_RANLIB


dnl ==============================================================
dnl Checks for header files.
dnl ==============================================================
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h libintl.h limits.h locale.h \
netdb.h netinet/in.h sys/file.h sys/ioctl.h \
sys/socket.h termios.h])

AC_CHECK_HEADER(pthread.h,, AC_MSG_ERROR([[
==================== PTHREAD HEADERS NOT FOUND
pthread.h needs to be present on your system in order
to compile the threading code. If you have this header,
make sure to make it reachable in the usual include path.
Otherwise it can be that POSIX threads are not supported
on the platform you are compiling. Usually this is a strange
behaviour on a unix platform, so you are invited to report
the problem over the MuSE mailinglist at muse@dyne.org
documenting your system configuration.
]]))

dnl ==============================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ==============================================================
AC_C_CONST
AC_HEADER_TIME
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

dnl ==============================================================
dnl Checks for library functions.
dnl ==============================================================
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit endhostent ftruncate getaddrinfo getcwd gethostbyname \
getnameinfo gettimeofday inet_aton inet_pton memset mkdir nanosleep scandir \
select sethostent setlocale socket strcasecmp strchr strdup strerror \
strncasecmp strstr strtol])

AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)

dnl =============
dnl CHECK MAT LIB
dnl =============
AC_CHECK_LIB(m, cos, M_LIBS="-lm", M_LIBS="")


dnl =================
dnl CHECK FOR GETTEXT
dnl =================
dnl removed until someone fixes the Makefile (TODO)
dnl the current mess with locales prevents from compiling
dnl and i couldn't figure out why // jrml 11apr05
dnl AM_GNU_GETTEXT([external])
dnl ALL_LINGUAS="it es"


dnl ====================
dnl CHECK FOR LIBSNDFILE
dnl ====================
PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.0,
                  [have_sndfile="yes"
                   AC_DEFINE(have_sndfile, 1, have working SNDFILE)],
                  [have_sndfile="no"])


dnl ====================
dnl CHECK FOR LIBJACK
dnl ====================
AC_ARG_ENABLE(jack,
  AS_HELP_STRING([--enable-jack],
                 [compile with Jack support http://jackaudio.org (no)]),
  [enable_jack=$enableval],
  [enable_jack=no])

if test x$enable_jack = xyes; then
  PKG_CHECK_MODULES(JACK, jack >= 0.34,
                    [have_jack="yes"
                     AC_DEFINE(HAVE_JACK, 1, have working jack)],
                    [have_jack="no"])
fi


dnl ==================
dnl ncurses GUI
dnl ==================
AC_ARG_ENABLE(ncurses,
    AS_HELP_STRING([--enable-ncurses],[compile with ncurses GUI (no)]),
    [want_ncurses_gui=$enableval],
    [want_ncurses_gui=no]) # disabled for now

# this is really libcdk's configuration, needs to be reviewed if correct
# see src/ncursesgui/libcdk
if test x$want_ncurses_gui = xyes; then
  AC_CHECK_LIB(termcap, waddstr,
               [have_termcap=yes], [have_termcap=no])

  AC_CHECK_LIB(ncurses, start_color,
               AC_DEFINE([HAVE_START_COLOR], 1, [Have start_color]))

  AC_CHECK_LIB(ncurses, initscr,
               [have_ncurses=yes], [have_ncurses=no])

  AC_CHECK_HEADERS([ncurses.h],
                   [have_ncurses_h=yes], [have_ncurses_h=no])

  AC_CHECK_FUNC(XCursesExit, AC_DEFINE(HAVE_XCURSES, 1, [Build with xcurses]))

  if test x$have_ncurses_h = xyes && test x$want_ncurses != xno; then
    AC_DEFINE([GUI_RUBIK], 1, [Have ncurses GUI])
  fi
fi


dnl ==================
dnl GTK2 GUI
dnl ==================
AC_ARG_ENABLE(gtk2,
    AS_HELP_STRING([--enable-gtk2],[compile with GTK2 GUI (auto)]),
    [want_gtk2_gui=$enableval],
    [want_gtk2_gui=auto])

if test x$want_gtk2_gui = xyes || test x$want_gtk2_gui = xauto; then
  PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 2.0.0 gthread-2.0 >= 2.0.0,
                    have_gtk2="yes", have_gtk2="no")

  if test x$want_gtk2_gui = xyes && test x$have_gtk2 = xno; then
    AC_MSG_ERROR([GTK2 GUI requested but GTK2 cannot be found!])
  fi

  PKG_CHECK_MODULES(GLIB2, glib-2.0 >= 2.0.0,
                    have_glib="yes", have_glib="no")

  if test x$want_gtk2_gui = xyes && test x$have_glib = xno; then
    AC_MSG_ERROR([GTK2 GUI requested but Glib2 cannot be found!])
  fi

  if test x$have_gtk2 = xyes && test x$have_glib = xyes; then
    want_gtk2_gui=yes
    AC_DEFINE(GUI_NIGHTOLO2, 1, [Have GTK2 GUI])
  else
    want_gtk2_gui=no
  fi
fi


dnl ===============
dnl CHECK OGG/VORBIS
dnl ===============
PKG_CHECK_MODULES(OGG, ogg,
                  [have_ogg=yes
                   AC_DEFINE(HAVE_OGG, 1, have ogg libraries)],
                  [have_ogg=no])

PKG_CHECK_MODULES(VORBIS, vorbis vorbisenc vorbisfile,
                  [have_vorbis=yes
                   AC_DEFINE(HAVE_VORBIS, 1, have vorbis audio libraries)],
                  [have_vorbis=no])


dnl ==============
dnl CHECK LAME LIB
dnl ==============
AC_CHECK_HEADER(lame/lame.h, [
        have_lame=yes;
        AC_MSG_NOTICE([MP3 encoder found and successfully linked])
	AC_DEFINE(HAVE_LAME, 1, [Building with LAME support])
	LAME_LIBS="-lmp3lame"
	AC_SUBST(LAME_LIBS)
                             ], [
        have_lame=no;
        AC_MSG_WARN([LAME libraries not found, no MP3 support])
                             ])


dnl Check for socklen_t
AC_MSG_CHECKING([for unix98 socklen_t])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
                #include <sys/socket.h>]], [[typedef socklen_t int;
                int test() { return 1; }]])],[AC_MSG_RESULT(yes)
               HAVE_SOCKLEN_T=yes],[AC_MSG_RESULT(no)
               HAVE_SOCKLEN_T=no])


dnl ===================
dnl assign some defines
dnl ===================

GUI_LIBS=""
GUI_DIRS=""
GUI_FLAGS=""
if test "$want_ncurses_gui" = "yes"; then
   GUI_LIBS="$GUI_LIBS ncursesgui/librubik.a ncursesgui/libcdk/libcdk.a"
   GUI_DIRS="$GUI_DIRS ncursesgui"
   GUI_FLAGS="$GUI_FLAGS -I\$(top_srcdir)/src/ncursesgui"
fi

if test "${want_gtk2_gui}" = "yes"; then
   GUI_LIBS="$GUI_LIBS $GTK2_LIBS gtkgui2/libnightolo2.a"
   GUI_DIRS="$GUI_DIRS gtkgui2"
   GUI_FLAGS="$GUI_FLAGS $GTK2_CFLAGS"

   dnl =========
   dnl Scheduler 
   dnl =========

dnl    scheduler is deactivated for now
dnl    AC_DEFINE(HAVE_SCHEDULER)

fi

dnl ==============================================================
dnl Choose external portaudio library
dnl =========================================
AC_MSG_CHECKING(if external portaudio library is requested)
PORTAUDIO_LIBS="\$(top_builddir)/lib/portaudio/libportaudio.a"
PORTAUDIO_CFLAGS="-I\$(top_srcdir)/lib/portaudio"
PORTAUDIO_DIRS="portaudio"
# add -lossaudio library needed for BSD systems
if test x$have_bsd = xyes; then
	PORTAUDIO_LIBS="$PORTAUDIO_LIBS -lossaudio"
fi
AC_ARG_WITH(portaudio,
[  --with-portaudio=/sw portaudio lib prefix],
[if test -n "$withval" && test -d "$withval" ; then
	 external_portaudio=yes
     PORTAUDIO_LIBS="$withval/lib/libportaudio.a -I$withval/pa_common "
	 PORTAUDIO_CFLAGS="-I$withval/include -I$withval/pa_common "
	 PORTAUDIO_DIRS=""
	 AC_MSG_RESULT(yes)
 fi],[AC_MSG_RESULT(no)])
AC_SUBST(PORTAUDIO_LIBS)
AC_SUBST(PORTAUDIO_CFLAGS)
AC_SUBST(PORTAUDIO_DIRS)


dnl ==============================================================
dnl mpeg lib
dnl ==============================================================
LIBMPEG_LIBS="\$(top_builddir)/lib/libmpeg/libmpeg.a"
LIBMPEG_CFLAGS="-I\$(top_srcdir)/lib/libmpeg"
AC_SUBST(LIBMPEG_LIBS)
AC_SUBST(LIBMPEG_CFLAGS)


dnl ==============================================================
dnl shout lib
dnl ==============================================================
PKG_CHECK_MODULES([SHOUT], [shout], [:],
  AC_MSG_ERROR([*** Libshout development files not found!]))


dnl ==============================================================
dnl samplerate lib
dnl ==============================================================
PKG_CHECK_MODULES([SAMPLERATE], [samplerate], [:],
  AC_MSG_ERROR([*** Libsamplerate development files not found!]))


dnl ==============================================================
dnl Detect possible output devices
dnl ==============================================================

dnl Check for OSS

AC_CHECK_HEADERS(sys/soundcard.h)
AC_CHECK_HEADERS(machine/soundcard.h)

dnl ==============================================================
dnl link with memory debugging library dmalloc
dnl ==============================================================
dnl godog: this can be replaced by AM_WITH_DMALLOC
AC_CHECK_HEADERS([dmalloc.h], [have_dmalloc=yes], [have_dmalloc=no])
AC_ARG_WITH(dmalloc,
  AS_HELP_STRING([--with-dmalloc],[use dmalloc, as in ftp://ftp.letters.com/src/dmalloc/dmalloc.tar.gz]),
  [with_dmalloc=$withval],
  [with_dmalloc=no])

if test x$with_dmalloc = xyes; then
  if test x$have_dmalloc = xyes; then
    AC_DEFINE(WITH_DMALLOC,1,[Define if using the dmalloc package])
    LIBS="$LIBS -ldmallocthcxx"
  else
    AC_MSG_ERROR([*** dmalloc requested but header not found!])
  fi
fi


dnl ==============================================================================
dnl CHECK to use profiling flags when compiling, for execution analysis with gprof
dnl ==============================================================================
AC_ARG_ENABLE(profiling,
  AS_HELP_STRING([--enable-profiling],
                 [compile using gprof flags for execution analysis (no)]),
  [enable_profiling=$enableval],
  [enable_profiling=no])

if test x$enable_profiling = xyes; then
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -pg"
fi

dnl ==============================================================
dnl CHECK to use processor specific optimizations to compile
dnl ==============================================================
AC_ARG_ENABLE(lubrify,
    AS_HELP_STRING([--enable-lubrify],
                   [compile using processor lubrifications (yes)]),
    [enable_lubrifications=$enableval],
    [enable_lubrifications=yes])

if test x$enable_lubrifications = xyes; then
  GLOBAL_CFLAGS="$GLOBAL_CFLAGS -O3 -fomit-frame-pointer -ffast-math"
fi


dnl ===============================
dnl declare things for the automake
dnl ===============================

AC_SUBST(DARWIN_LIBS)
AC_SUBST(DARWIN_CFLAGS)

AC_SUBST(M_LIBS)
AC_SUBST(LAME_LIBS)

AC_SUBST(GUI_LIBS)
AC_SUBST(GUI_DIRS)
AC_SUBST(GUI_FLAGS)
AC_SUBST(GUI_DEPS)

CFLAGS="$GLOBAL_CFLAGS $CFLAGS"
CXXFLAGS="$GLOBAL_CFLAGS $CXXFLAGS"
LIBS="$LIBS"

AC_CONFIG_FILES([
Makefile
lib/Makefile
lib/libmpeg/Makefile
lib/portaudio/Makefile
src/Makefile
src/gtkgui2/Makefile
src/ncursesgui/Makefile
src/ncursesgui/libcdk/Makefile
])
AC_OUTPUT

dnl locales removed
dnl intl/Makefile
dnl po/Makefile

echo

echo "== MuSE build configuration ====================================="
echo ":: Building on ${host}"

if test "$have_darwin" = "yes"; then
	echo ":: Linking Darwin libs $DARWIN_LIBS"
fi


if test "$have_ogg" = "yes"; then
	echo ":: Linking Ogg libraries: $OGG_LIBS"
else
	echo "OGG library is needed, go and install it from xiph.org"
fi

if test "$have_vorbis" = "yes"; then
	echo ":: Linking Vorbis libraries: $VORBIS_LIBS"
else
	echo "VORBIS library is needed, go and install it from xiph.org"
fi

if test "${have_sndfile}" = "yes"; then
	echo ":: Linking SndFile libraries for WAV/AIFF/VOC/etc encoding"
fi

if test "${have_jack}" = "yes"; then
	echo ":: Linking libJack for connecting to the Jack audio daemon"
fi

if test "$have_lame" = "yes"; then
	echo ":: Linking Lame libraries for MP3 encoding"
fi

if test "${want_gtk2_gui}" = "yes"; then
	echo ":: Building the GTK-2 user interface"
fi

if test "${want_ncurses_gui}" = "yes"; then
	echo ":: Building the NCURSES user interface"
fi
if test "$external_portaudio" = "yes"; then
	echo ":: Embedding external portaudio from $PORTAUDIO_LIBS"
fi

if test "$enable_profiling" = "yes"; then
        echo ":: PROFILING informations ON (to use with gprof)"
else
        echo ":: PROFILING informations OFF (see man gprof)"
fi

if test "$with_dmalloc" = "yes"; then
	echo ":: Using dmalloc for memory fencing and debugging"
fi

echo ":: CFLAGS = $CFLAGS"
echo ":: CXXFLAGS = $CXXFLAGS"
echo ":: LIBS = $LIBS"
echo "============================== now type make, may the source be with you!"
echo
