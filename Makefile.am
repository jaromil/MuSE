if NCURSES_GUI
LIBCDKDIR = src/ncursesgui/libcdk
endif

SUBDIRS = lib/libmpeg $(LIBCDKDIR)

dist_docs_DATA = README COPYING AUTHORS NEWS KNOWN-BUGS ChangeLog TODO USAGE
docsdir = ${prefix}/share/doc/${PACKAGE}

dist_man_MANS = doc/muse.1

ACLOCAL_AMFLAGS = -I m4

# initialize some vars, append later
noinst_HEADERS =
noinst_LIBRARIES =
noinst_DATA =
EXTRA_DIST =

# FIXME po should be properly gettexited
EXTRA_DIST += autogen.sh po utils

### src/
bin_PROGRAMS = src/muse
src_muse_SOURCES = \
  src/muse.cpp \
  src/jmixer.cpp \
  src/audioproc.cpp \
  src/inchannels.cpp \
  src/decoder.cpp \
  src/dec_mp3.cpp \
  src/dec_ogg.cpp \
  src/dec_snd.cpp \
  src/dec_jack.cpp \
  src/outchannels.cpp \
  src/out_vorbis.cpp \
  src/out_lame.cpp \
  src/pipe.cpp \
  src/shouter.cpp \
  src/linklist.cpp \
  src/playlist.cpp \
  src/jutils.cpp \
  src/radiosched.cpp \
  src/httpstream.cpp \
  src/gui.cpp \
  src/dev_sound.cpp \
  src/lame_wrap.cpp

src_muse_CXXFLAGS = \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/lib/libmpeg \
  $(DARWIN_CFLAGS) \
  $(GLIB2_CFLAGS) \
  $(GUI_FLAGS) \
  $(JACK_CFLAGS) \
  $(OGG_CFLAGS) \
  $(PORTAUDIO_CFLAGS) \
  $(SAMPLERATE_CFLAGS) \
  $(SHOUT_CFLAGS) \
  $(VORBIS_CFLAGS)

src_muse_LDADD = \
  $(top_builddir)/lib/libmpeg/libmpeg.a \
  -lpthread \
  -ldl \
  $(DARWIN_LIBS) \
  $(GLIB2_LIBS) \
  $(GUI_LIBS) \
  $(JACK_LIBS) \
  $(M_LIBS) \
  $(OGG_LIBS) \
  $(PORTAUDIO_LIBS) \
  $(SAMPLERATE_LIBS) \
  $(SHOUT_LIBS) \
  $(SNDFILE_LIBS) \
  $(VORBIS_LIBS)

noinst_HEADERS += \
  src/audioproc.h \
  src/dec_jack.h \
  src/dec_mp3.h \
  src/decoder.h \
  src/dec_ogg.h \
  src/dec_snd.h \
  src/dev_sound.h \
  src/generic.h \
  src/gui.h \
  src/httpstream.h \
  src/inchannels.h \
  src/jmixer.h \
  src/jutils.h \
  src/lame_wrap.h \
  src/linklist.h \
  src/outchannels.h \
  src/out_lame.h \
  src/out_vorbis.h \
  src/pipe.h \
  src/playlist.h \
  src/profile.h \
  src/radiosched.h \
  src/resarr160to441.h \
  src/resarr320to441.h \
  src/shouter.h \
  src/xmlprofile.h

EXTRA_DIST += src/vumeter.c src/xmlprofile.cpp src/profile.cpp


## src/gtkgui2
if GTK2_GUI
noinst_LIBRARIES += src/gtkgui2/libnightolo2.a
src_gtkgui2_libnightolo2_a_SOURCES = \
  src/gtkgui2/about.cpp \
  src/gtkgui2/docking.cpp \
  src/gtkgui2/encoder.cpp \
  src/gtkgui2/filedump.cpp \
  src/gtkgui2/gchan.cpp \
  src/gtkgui2/gchan_events.cpp \
  src/gtkgui2/gradiosched.cpp \
  src/gtkgui2/gtk2_gui.cpp \
  src/gtkgui2/gtkdial.cpp \
  src/gtkgui2/ice.cpp \
  src/gtkgui2/listpack.cpp \
  src/gtkgui2/musegtk.cpp \
  src/gtkgui2/utils.cpp \
  src/gtkgui2/vumeters.cpp \
  src/gtkgui2/xmlprofile.cpp

# FIXME why xmlprofile.h is duplicated into src/gtkgui2 and src/ ?
src_gtkgui2_libnightolo2_a_CXXFLAGS = \
  -DLOCALEDIR=\"$(localedir)\" \
  $(GTK2_CFLAGS) \
  -I$(top_srcdir)/src/gtkgui2 \
  -I$(top_srcdir)/src

noinst_HEADERS += \
  src/gtkgui2/about.h \
  src/gtkgui2/docking.h \
  src/gtkgui2/encoder.h \
  src/gtkgui2/filedump.h \
  src/gtkgui2/gchan_events.h \
  src/gtkgui2/gchan.h \
  src/gtkgui2/gen.h \
  src/gtkgui2/gradiosched.h \
  src/gtkgui2/gtk2_gui.h \
  src/gtkgui2/gtkdial.h \
  src/gtkgui2/ice.h \
  src/gtkgui2/listpack.h \
  src/gtkgui2/musegtk.h \
  src/gtkgui2/utils.h \
  src/gtkgui2/vumeters.h \
  src/gtkgui2/xmlprofile.h

EXTRA_DIST += src/gtkgui2/xpm2
endif


## src/ncursesgui
if NCURSES_GUI
noinst_LIBRARIES += src/ncursesgui/librubik.a
src_ncursesgui_librubik_a_SOURCES = \
  src/ncursesgui/entry.cpp \
  src/ncursesgui/fselect.cpp \
  src/ncursesgui/label.cpp \
  src/ncursesgui/muse_tui.cpp \
  src/ncursesgui/play_infobox.cpp \
  src/ncursesgui/screen.cpp \
  src/ncursesgui/scroll.cpp \
  src/ncursesgui/single_channel.cpp \
  src/ncursesgui/slider.cpp \
  src/ncursesgui/about.cpp \
  src/ncursesgui/connectbox.cpp \
  src/ncursesgui/encsetbox.cpp \
  src/ncursesgui/streamsetbox.cpp \
  src/ncursesgui/itemlist.cpp \
  src/ncursesgui/menu.cpp \
  src/ncursesgui/ncurses_gui.cpp

src_ncursesgui_librubik_a_CXXFLAGS = \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/ncursesgui

noinst_HEADERS += \
  src/ncursesgui/about.h \
  src/ncursesgui/connectbox.h \
  src/ncursesgui/debug.h \
  src/ncursesgui/encdata.h \
  src/ncursesgui/encsetbox.h \
  src/ncursesgui/entry.h \
  src/ncursesgui/fselect.h \
  src/ncursesgui/itemlist.h \
  src/ncursesgui/label.h \
  src/ncursesgui/menu.h \
  src/ncursesgui/muse_console.h \
  src/ncursesgui/muse_tui.h \
  src/ncursesgui/ncurses_gui.h \
  src/ncursesgui/play_infobox.h \
  src/ncursesgui/screen.h \
  src/ncursesgui/scroll.h \
  src/ncursesgui/single_channel.h \
  src/ncursesgui/slider.h \
  src/ncursesgui/streamsetbox.h

EXTRA_DIST += \
  src/ncursesgui/coltable.c \
  src/ncursesgui/muse_console.cpp \
  src/ncursesgui/testgui.sh
endif


## doc/
if BUILD_DOC
installdest = $(DESTDIR)$(docdir)/

doc/html/index.html: doc/muse.doxygen
	$(DOXYGEN) doc/muse.doxygen

noinst_DATA += doc/html/index.html

install-data-local:
	$(INSTALL) -d $(installdest)
	(cd doc && for d in `find html -type d`; do $(INSTALL) -m755 -d $(installdest)/$$d; done)
	(cd doc && for f in `find html -type f`; do $(INSTALL) -m644 $$f $(installdest)/$$f; done)

uninstall-local:
	rm -rf $(installdest)/html

clean-local:
	rm -rf doc/html doc/latex doc/rtf doc/man

EXTRA_DIST += doc/muse.doxygen.in
endif
