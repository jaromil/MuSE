SUBDIRS = @GUI_DIRS@

DIST_SUBDIRS = gtkgui2 ncursesgui

MUSE_CFLAGS = @GUI_FLAGS@ @JACK_CFLAGS@ @OGG_CFLAGS@ @VORBIS_CFLAGS@ \
              @GLIB2_CFLAGS@ @DARWIN_CFLAGS@ @PORTAUDIO_CFLAGS@ @SHOUT_CFLAGS@ \
              @VORBIS_CFLAGS@ @OGG_CFLAGS@ @SAMPLERATE_CFLAGS@ @LIBMPEG_CFLAGS@

AM_CFLAGS = $(MUSE_CFLAGS)
AM_CXXFLAGS = $(MUSE_CFLAGS)

EXTRA_DIST = vumeter.c xmlprofile.cpp profile.cpp

noinst_HEADERS = audioproc.h dec_jack.h dec_mp3.h decoder.h dec_ogg.h dec_snd.h \
  dev_sound.h generic.h gui.h httpstream.h inchannels.h jmixer.h jutils.h \
  linklist.h outchannels.h out_lame.h out_vorbis.h pipe.h playlist.h profile.h \
  radiosched.h resarr160to441.h resarr320to441.h shouter.h xmlprofile.h \
  lame_wrap.h

bin_PROGRAMS = muse

muse_SOURCES =  \
    muse.cpp        	\
    jmixer.cpp      	\
    audioproc.cpp       \
    inchannels.cpp      \
    decoder.cpp		\
    dec_mp3.cpp		\
    dec_ogg.cpp		\
    dec_snd.cpp		\
    dec_jack.cpp	\
    outchannels.cpp     \
    out_vorbis.cpp	\
    out_lame.cpp	\
    pipe.cpp        	\
    shouter.cpp     	\
    linklist.cpp        \
    playlist.cpp        \
    jutils.cpp      	\
    radiosched.cpp      \
    httpstream.cpp  	\
    gui.cpp 		\
    dev_sound.cpp \
    lame_wrap.cpp

muse_LDADD = \
	-lpthread \
	-ldl \
	@SAMPLERATE_LIBS@ \
	@M_LIBS@ \
	@OGG_LIBS@ \
	@VORBIS_LIBS@ \
	@GUI_LIBS@ \
	@SNDFILE_LIBS@ \
	@GLIB2_LIBS@ \
	@JACK_LIBS@ \
	@DARWIN_LIBS@ \
	@PORTAUDIO_LIBS@ \
	@LIBMPEG_LIBS@ \
	@SHOUT_LIBS@
