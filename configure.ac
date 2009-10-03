dnl ==============================================================
dnl Process this file with autoconf to produce a configure script.
dnl ==============================================================

AC_INIT([MuSE], [0.9.1-cvs])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/muse.cpp])

dnl ==============================================================
dnl Setup for automake
dnl ==============================================================

AM_INIT_AUTOMAKE
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

     CPUFLAGS=`cat /proc/cpuinfo | grep flags`
     if grep "^flags.* mmx" /proc/cpuinfo > /dev/null; then
       have_mmx=yes
       AC_DEFINE(HAVE_MMX,1,[define if enabling MMX acceleration])
     fi
     if grep "^flags.* sse" /proc/cpuinfo > /dev/null; then
       have_sse=yes
       AC_DEFINE(HAVE_SSE,1,[define if enabling SSE acceleration])
     fi
     ;;

     *darwin*)
     AC_MSG_RESULT([Darwin/OSX])
     AC_DEFINE(HAVE_DARWIN,1,[define if compiling for Apple Darwin OSX])
     have_darwin=yes
     DARWIN_LIBS="-L/sw/lib -lintl -framework AudioToolbox -framework CoreAudio"
     DARWIN_CFLAGS="-I/sw/include"
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


dnl ==============================================================
dnl Check for tools
dnl ==============================================================
AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_PROG_RANLIB
AC_PROG_MAKE_SET

dnl ==============================================================
dnl Add the local include path and some flags - only linux i*86 supported
dnl ==============================================================
#AC_SUBST(OSTYPE)
#AM_DEFINE_UNQUOTED(HOST_CPU, "${host_cpu}")

#CFLAGS="-O4 -D_REENTRANT -fomit-frame-pointer -march=${host_cpu} -ffast-math -malign-double -funroll-loops -pipe"
CFLAGS="-pipe -D_REENTRANT"

dnl ==============================================================
dnl Checks for header files.
dnl ==============================================================
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([fcntl.h sys/file.h sys/time.h unistd.h dirent.h sys/ndir.h \
                  sys/dir.h ndir.h limits.h unistd.h stdint.h inttypes.h])

AC_CHECK_HEADER(pthread.h,, [
	echo
	echo "==================== PTHREAD HEADERS NOT FOUND"
	echo "pthread.h needs to be found on your system in order"
	echo "to compile the threading code. If you have this header,"
	echo "make sure to make it reachable in the usual include path."
	echo "Otherwise it can be that POSIX threads are not supported"
	echo "on the platform you are compiling. Usually this is a strange"
	echo "behaviour on a unix platform, so you're invited to report"
	echo "the problem over the MuSE mailinglist at muse@dyne.org"
	echo "documenting your system configuration."
])

dnl ==============================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ==============================================================
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_MODE_T
AC_TYPE_SIGNAL
AC_HEADER_TIME

dnl ==============================================================
dnl Checks for library functions.
dnl ==============================================================
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gettimeofday select socket strdup nanosleep inet_aton scandir \
                getcwd mktime strerror sethostent endhostent getnameinfo \
                getaddrinfo inet_pton])

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
dnl AM_GNU_GETTEXT
dnl ALL_LINGUAS="it es"

dnl ====================
dnl CHECK FOR LIBSNDFILE
dnl ====================
PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.0 , [
    HAVE_SNDFILE="yes"
    AC_DEFINE(HAVE_SNDFILE, 1, have working SNDFILE)],
    HAVE_SNDFILE="no")

dnl ====================
dnl CHECK FOR LIBJACK
dnl ====================
AC_ARG_ENABLE(jack,
[  --enable-jack            compile with Jack-it support from http://jackit.sf.net (no)],
[ if test ! x$enableval = xyes; then
    use_jack=no
  else
    use_jack=yes;
  fi ],[ use_jack=no ])
JACK_LIBS=""
JACK_CFLAGS=""
if test x$use_jack = xyes; then
  PKG_CHECK_MODULES(JACK, jack >= 0.34, [
      HAVE_JACK="yes"
      AC_DEFINE(HAVE_JACK, 1, have working jack audio daemon and libjack)
  ], HAVE_JACK="no")
fi
AC_SUBST(JACK_LIBS)
AC_SUBST(JACK_CFLAGS)


dnl ====================
dnl switch for rubik GUI
dnl ====================
#AC_MSG_CHECKING(to build ncurses GUI by rubik)
#AC_ARG_WITH(rubik,
#[  --with-rubik        compile with ncurses CONSOLE GUI ],
#[if test "$withval" = yes; then
#  AC_MSG_RESULT(yes)
AC_CHECK_LIB(termcap, waddstr)
AC_CHECK_LIB(ncurses, start_color)
have_ncurses=yes
if test "$ac_cv_lib_ncurses_start_color" != yes ; then
  AC_CHECK_LIB(curses, initscr, have_ncurses=yes, have_ncurses=no)
fi

# ncurses now disable by default (needs more fixing)
want_ncurses=no
AC_ARG_ENABLE(ncurses,
[  --enable-ncurses        compile with ncurses interface (no)],
[  want_ncurses=$enableval])

if test "$have_ncurses" = yes -a ! x$want_ncurses = xno; then
  AC_DEFINE(HAVE_COLOR, 1, [Build with ncurses color support])
  AC_CHECK_FUNC(XCursesExit, AC_DEFINE(HAVE_XCURSES, 1, [Build with xcurses]))
  GUI_RUBIK=true
  AC_DEFINE([GUI_RUBIK], 1, [Have ncurses GUI])
fi
#  else
#  AC_MSG_RESULT(no)
#  have_ncurses=no
#fi], [AC_MSG_RESULT(no)])


dnl =======================
dnl switch for nightolo GUI
dnl =======================
dnl AC_ARG_ENABLE(nightolo,
dnl 	[  --enable-gtk		    compile with GTK+ graphical interface (autodetect)],
dnl	GUI_NIGHTOLO="true", GUI_NIGHTOLO="false")
dnl	[case "${enableval}" in
dnl	        yes) nightolo=true ;;
dnl		no) nightolo=false ;;
dnl		*) AC_MSG_ERROR(--enable-gtk expects either yes or no) ;;
dnl	esac], [nightolo=false])

dnl AM_CONDITIONAL(NIGHTOLO, test x$nightolo = xtrue)
dnl ===================
dnl CHECK FOR X Windows
dnl // this one i took from the Emacs configure.in
dnl ===================

AC_PATH_X
if test "$no_x" = yes; then
   window_system=none
else
   window_system=x11
fi
if test "${x_libraries}" != NONE && test -n "${x_libraries}"; then
  LD_SWITCH_X_SITE=-L`echo ${x_libraries} | sed -e "s/:/ -L/g"`
  LD_SWITCH_X_SITE_AUX=-R`echo ${x_libraries} | sed -e "s/:/ -R/g"`
  x_default_search_path=""
  for x_library in `echo ${x_libraries} | sed -e "s/:/ /g"`; do
    x_search_path="\
${x_library}/X11/%L/%T/%N%C%S:\
${x_library}/X11/%l/%T/%N%C%S:\
${x_library}/X11/%T/%N%C%S:\
${x_library}/X11/%L/%T/%N%S:\
${x_library}/X11/%l/%T/%N%S:\
${x_library}/X11/%T/%N%S"
    if test x"${x_default_search_path}" = x; then
      x_default_search_path=${x_search_path}
    else
      x_default_search_path="${x_search_path}:${x_default_search_path}"
    fi
  done
fi
if test "${x_includes}" != NONE && test -n "${x_includes}"; then
  C_SWITCH_X_SITE=-I`echo ${x_includes} | sed -e "s/:/ -I/g"`
fi
case "${window_system}" in
  x11 )
    HAVE_X11=yes
  ;;
  none )
    HAVE_X11=no
  ;;
esac

dnl ==================
dnl CHECK GTK AND GLIB
dnl ==================
#AC_MSG_CHECKING(to build new Gtk2 GUI by nightolo)
#AC_ARG_WITH(gtk2,
#[  --with-gtk2        force to compile with Gtk2 GUI ],
#[if test "$withval" = yes; then
#  AC_MSG_RESULT(yes)
#  FORCE_GTK2=yes
#fi], [AC_MSG_RESULT(no)])

AC_ARG_ENABLE(gtk2,
[  --enable-gtk2           compile with GTK2 gui (auto)],
[  want_gtk2=$enableval])

if test "${HAVE_X11}" = "yes"; then
   
PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 2.0.0 gthread-2.0 >= 2.0.0, [
  		  HAVE_GTK2="yes"
    		  AC_DEFINE(HAVE_GTK2, 1, have working GTK2)],
  		  HAVE_GTK2="no")

# gtk2 interface is preferred now!
  if test x$want_gtk2 = xyes; then
# GTK2 enabled
    HAVE_GTK=no
  elif test x$want_gtk2 = xno; then
# GTK2 disabled
    HAVE_GTK2=no
  elif test  "${HAVE_GTK2}" = "yes" -a "${HAVE_GTK}" = "yes" ; then
# both GTK and GTK2 are present, GTK2 is preferred
    HAVE_GTK="no"
  fi

  if test  "${HAVE_GTK}" = "yes" ; then
    GUI_NIGHTOLO=true
    AC_DEFINE(GUI_NIGHTOLO, 1, [Have GTK GUI])
  fi
  if test  "${HAVE_GTK2}" = "yes" ; then
    GUI_NIGHTOLO2=true
    AC_DEFINE(GUI_NIGHTOLO2, 1, [Have GTK2 GUI])
  fi
fi

dnl ===============
dnl CHECK OGGVORBIS
dnl ===============
PKG_CHECK_MODULES(OGG, ogg, [
	HAVE_OGG=yes
	AC_DEFINE(HAVE_OGG, 1, have ogg libraries)],
	HAVE_OGG=no)
AC_SUBST(OGG_LIBS)
PKG_CHECK_MODULES(VORBIS, vorbis vorbisenc vorbisfile, [
	HAVE_VORBIS=yes
	AC_DEFINE(HAVE_VORBIS, 1, have vorbis audio codec libraries)],
	HAVE_VORBIS=no)
AC_SUBST(VORBIS_LIBS)


dnl ==============
dnl CHECK LAME LIB
dnl ==============
AC_CHECK_HEADER(lame/lame.h, [
        have_lame=yes;
        AC_MSG_NOTICE([MP3 encoder found and succesfully linked])
	AC_DEFINE(HAVE_LAME, 1, [Building with LAME support])
	LAME_LIBS="-lmp3lame"
	AC_SUBST(LAME_LIBS)
                             ], [
        have_lame=no;
        AC_MSG_WARN([LAME libraries not found, no MP3 support])
                             ])

dnl Check for socklen_t
AC_MSG_CHECKING([for unix98 socklen_t])
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>],
               [typedef socklen_t int;
                int test() { return 1; }],
               AC_MSG_RESULT(yes)
               HAVE_SOCKLEN_T=yes,
               AC_MSG_RESULT(no)
               HAVE_SOCKLEN_T=no)

dnl ===================
dnl assign some defines
dnl ===================

if test "${HAVE_X11}" = "yes" ; then
  AC_DEFINE(HAVE_X11, 1, [Build with X11 support])
fi

GUI_LIBS=""
GUI_DIRS=""
GUI_FLAGS=""
GUI_DEPS=""
if test "${GUI_RUBIK}" = "true"; then
   GUI_LIBS="$GUI_LIBS ncursesgui/librubik.a ncursesgui/libcdk/libcdk.a"
   GUI_DIRS="$GUI_DIRS ncursesgui"
   GUI_FLAGS="$GUI_FLAGS"
   GUI_DEPS="$GUI_DEPS ncursesgui/librubik.a"
   AC_DEFINE(HAVE_LIBNCURSES, 1, "have working libncurses")
fi

if test "${HAVE_GTK}" = "yes"; then
   GUI_LIBS="$GUI_LIBS $GTK_LIBS -lgthread gtkgui/libnightolo.a"
   GUI_DIRS="$GUI_DIRS gtkgui"
   GUI_FLAGS="$GUI_FLAGS $GTK_FLAGS"
   GUI_DEPS="$GUI_DEPS gtkgui/libnightolo.a"
fi

if test "${HAVE_GTK2}" = "yes"; then
   GUI_LIBS="$GUI_LIBS $GTK2_LIBS gtkgui2/libnightolo2.a"
   GUI_DIRS="$GUI_DIRS gtkgui2"
   GUI_FLAGS="$GUI_FLAGS $GTK2_CFLAGS"
   GUI_DEPS="$GUI_DEPS gtkgui2/libnightolo2.a"

   dnl =========
   dnl Scheduler 
   dnl =========

dnl    scheduler is deactivated for now
dnl    AC_DEFINE(HAVE_SCHEDULER)

   PKG_CHECK_MODULES(GLIB2, glib-2.0 >= 2.0.0,,
    AC_MSG_ERROR(glib-2.0 not found))
fi

dnl ==============================================================
dnl Choose external portaudio library
dnl =========================================
AC_MSG_CHECKING(if external portaudio library is requested)
PORTAUDIO_LIBS="portaudio/libportaudio.a"
PORTAUDIO_CFLAGS="-Iportaudio"
PORTAUDIO_DIRS="portaudio"
# add -lossaudio library needed for BSD systems
if test $have_bsd = yes; then
	PORTAUDIO_LIBS="$PORTAUDIO_LIBS -lossaudio"
fi
AC_ARG_WITH(portaudio,
[  --with-portaudio=/sw portaudio lib prefix],
[if test -r "$withval"; then
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
dnl Choose which libshout version to use
dnl ==============================================================
dnl LIBSHOUT_DIR=shout
AC_MSG_CHECKING(where to link libshout... )
LIBSHOUT_LIBS="shout/libshout.a"
LIBSHOUT_DIR="shout"
LIBSHOUT_CFLAGS=""

AC_ARG_WITH(shared-shout,
[  --with-shared-shout    link to system shared libshout library],
[  AC_MSG_RESULT(system shared libraries) 
   PKG_CHECK_MODULES(LIBSHOUT, shout >= 2.0, [
    system_libshout=yes
    LIBSHOUT_DIR=""],[ system_libshout=no ]) ], system_libshout=no)

AC_ARG_WITH(static-shout,
[  --with-static-shout=/libshout source directory 
              is compiled to embed static thread-disabled version],
[if test -r "$withval"; then
    LIBSHOUT_SRCDIR="`find $withval -name 'src'`"
    LIBSHOUT_LIBS="$LIBSHOUT_SRCDIR/.libs/libshout.a"
    if test ! -r "$LIBSHOUT_LIBS"; then
        AC_MSG_RESULT(external static not found, using internal)
	AC_MSG_WARN([!! $LIBSHOUT_LIBS not found: you must compile the static library
               of libshout])
	LIBSHOUT_LIBS="shout/libshout.a"
	LIBSHOUT_DIR="shout"
    else
	LIBSHOUT_DIR=""
	external_libshout=yes
	AC_MSG_RESULT(external static)
    fi
 fi
],[AC_MSG_RESULT(libshout configured)])
AC_SUBST(LIBSHOUT_DIR)
AC_SUBST(LIBSHOUT_LIBS)
AC_SUBST(LIBSHOUT_CFLAGS)

dnl ==============================================================
dnl Detect CPU clipping mode (from the secret rabbit code)
dnl ==============================================================
AC_C_CLIP_MODE
AC_DEFINE_UNQUOTED(CPU_CLIPS_POSITIVE, ${ac_cv_c_clip_positive},
	[Target processor clips on positive float to int conversion.])
AC_DEFINE_UNQUOTED(CPU_CLIPS_NEGATIVE, ${ac_cv_c_clip_negative},
	[Target processor clips on negative float to int conversion.])

dnl ==============================================================
dnl check for lrint and lrintf functions
dnl ==============================================================
AC_C99_FUNC_LRINT
AC_C99_FUNC_LRINTF


dnl ==============================================================
dnl Detect possible output devices 
dnl ==============================================================

dnl Check for OSS

AC_CHECK_HEADERS(sys/soundcard.h)
AC_CHECK_HEADERS(machine/soundcard.h)
AM_CONDITIONAL(HAVE_OSS,test "${ac_cv_header_sys_soundcard_h}" = "yes" || test "${ac_cv_header_machine_soundcard_h}" = "yes")

dnl ==============================================================
dnl link with memory debugging library dmalloc
dnl ==============================================================
AC_CHECK_HEADER(dmalloc.h)
if test "$ac_cv_header_dmalloc_h" = "yes"; then

AC_MSG_CHECKING(if malloc debugging is wanted)
AC_ARG_WITH(dmalloc,
[  --with-dmalloc          use dmalloc, as in
                          ftp://ftp.letters.com/src/dmalloc/dmalloc.tar.gz],
[if test "$withval" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(WITH_DMALLOC,1,
            [Define if using the dmalloc debugging malloc package])
  LIBS="$LIBS -ldmallocthcxx"
  DMALLOCC="dmallocc.cpp"
  have_dmalloc=yes
  debug=true
else
  AC_MSG_RESULT(no)
  DMALLOCC=""
  have_dmalloc=no
fi], [AC_MSG_RESULT(no)])

fi

dnl ==============================================================
dnl compile with full warnings and debugging symbols
dnl ==============================================================
AC_ARG_ENABLE(debug,
	[  --enable-debug          compile with debug symbols (-Wall -g) [yes]],
	[case "${enableval}" in
		yes) debug=true ;;
		no) debug=false ;;
		*) AC_MSG_ERROR(--enable-debug expects either yes or no) ;;
	esac], [debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)
if test x$debug = xtrue; then
   CFLAGS="$CFLAGS -Wall -g"
fi




dnl ==============================================================================
dnl CHECK to use profiling flags when compiling, for execution analysis with gprof
dnl ==============================================================================
AC_ARG_ENABLE(profiling,
[  --enable-profiling      compile using gprof flags for execution analysis (no)],
[ if test ! x$enableval = xyes; then
    have_profiling=no
  else
      have_profiling=yes;
  fi ],[ have_profiling=no ])
if test x$have_profiling = xyes; then
  CFLAGS="$CFLAGS -pg"
fi

dnl ==============================================================
dnl CHECK to use processor specific optimizations to compile
dnl ==============================================================
AC_ARG_ENABLE(lubrify,
[  --enable-lubrify        compile using processor lubrifications (yes)],
[ if test ! x$enableval = xyes; then
    have_lubrifications=no
  else
    have_lubrifications=yes;
  fi ],[ have_lubrifications=yes ])
if test x$have_lubrifications = xyes; then
#  LUBRIFY_CFLAGS="-O4 -fomit-frame-pointer -march=${host_cpu} -finline-functions -ffast-math -funroll-loops"
   if test x$have_profiling = xyes -o x$debug = xtrue; then
     AC_MSG_WARN([cannot lubrify code if debug or profiling are enabled])
     CFLAGS="$CFLAGS -ggdb"
   else
     CFLAGS="$CFLAGS -O3 -fomit-frame-pointer -ffast-math"
     if test x$have_darwin = xyes; then
	CFLAGS="$CFLAGS -mabi=altivec -maltivec"
     fi
   fi
else
   CFLAGS="$CFLAGS -O2 -ffast-math"
fi




dnl ===============================
dnl declare things for the automake
dnl ===============================

AC_SUBST(DARWIN_LIBS)
AC_SUBST(DARWIN_CFLAGS)

AC_SUBST(M_LIBS)
AC_SUBST(LAME_LIBS)
AC_SUBST(OGG_LIBS)
AC_SUBST(VORBIS_LIBS)

AC_SUBST(GUI_LIBS)
AC_SUBST(GUI_DIRS)
AC_SUBST(GUI_FLAGS)
AC_SUBST(GUI_DEPS)

AC_SUBST(LIBSHOUT_LIBS)
AC_SUBST(LIBSHOUT_DIR)

AC_SUBST(DMALLOCC)
AC_SUBST(LUBRIFY_CFLAGS)

CXXFLAGS="$CFLAGS"
LIBS="$LIBS"

AC_OUTPUT([
Makefile
src/Makefile
src/shout/Makefile
src/libmpeg/Makefile
src/gtkgui/Makefile
src/gtkgui2/Makefile
src/ncursesgui/Makefile
src/ncursesgui/libcdk/Makefile
src/resample/Makefile
src/portaudio/Makefile
])

dnl locales removed
dnl intl/Makefile
dnl po/Makefile

echo

echo "== MuSE build configuration ====================================="
echo ":: Building on ${host}"

if test "$have_darwin" = "yes"; then
	echo ":: Linking Darwin libs $DARWIN_LIBS"
fi


if test "$HAVE_OGG" = "yes"; then
	echo ":: Linking Ogg libraries: $OGG_LIBS"
else
	echo "OGG library is needed, go and install it from xiph.org"
fi

if test "$HAVE_VORBIS" = "yes"; then
	echo ":: Linking Vorbis libraries: $VORBIS_LIBS"
else
	echo "VORBIS library is needed, go and install it from xiph.org"
fi

if test "${HAVE_SNDFILE}" = "yes"; then
	echo ":: Linking SndFile libraries for WAV/AIFF/VOC/etc encoding"
fi

if test "${HAVE_JACK}" = "yes"; then
	echo ":: Linking libJack for connecting to the Jack audio daemon"
fi

if test "$have_lame" = "yes"; then
	echo ":: Linking Lame libraries for MP3 encoding"
fi

if test "${GUI_NIGHTOLO2}" = "true"; then
	echo ":: Building the GTK-2 user interface"
fi

if test "${GUI_RUBIK}" = "true"; then
	echo ":: Building the NCURSES user interface"
fi
if test "$external_libshout" = "yes"; then
	echo ":: Embedding external libshout from $LIBSHOUT_LIBS"
fi
if test "$system_libshout" = "yes"; then
	echo ":: Linking to system shared libshout"
fi
if test "$external_portaudio" = "yes"; then
	echo ":: Embedding external portaudio from $PORTAUDIO_LIBS"
fi
if test "$debug" = "true"; then
	echo ":: DEBUG informations ON (-Wall -g compile flags)"
else
	echo ":: DEBUG informations OFF"
fi

if test "$have_profiling" = "yes"; then
        echo ":: PROFILING informations ON (to use with gprof)"
else
        echo ":: PROFILING informations OFF (see man gprof)"
fi

if test "$have_dmalloc" = "yes"; then
	echo ":: Using dmalloc for memory fencing and debugging"
fi

echo ":: CFLAGS = $CFLAGS"
echo ":: CXXFLAGS = $CXXFLAGS"
echo ":: LIBS = $LIBS"
echo "============================== now type make, may the source be with you!"
echo
