SUBDIRS = resample shout portaudio @GUI_DIRS@

INCLUDES = -Ishout -Iportaudio -I/usr/X11R6/include -Incursesgui @GLIB2_CFLAGS@

CFLAGS = @GUI_FLAGS@  

bin_PROGRAMS = muse

muse_SOURCES =  \
    muse.cpp        \
    jmixer.cpp      \
    audioproc.cpp       \
    inchannels.cpp      \
    decoder.cpp		\
    dec_mp3.cpp		\
    dec_ogg.cpp		\
    dec_snd.cpp		\
    outchannels.cpp     \
    out_vorbis.cpp	\
    out_lame.cpp	\
    pipe.cpp        \
    shouter.cpp     \
    linklist.cpp        \
    playlist.cpp        \
    jutils.cpp      \
    radiosched.cpp      \
    httpstream.cpp  \
    gui.cpp \
    dev_sound.cpp

muse_DEPENDENCIES = \
	@GUI_DEPS@ \
	resample/libresample.a \
	shout/libshout.a \
	portaudio/libportaudio.a

muse_LDADD = \
	$(top_srcdir)/src/resample/libresample.a \
	$(top_srcdir)/src/shout/libshout.a \
	$(top_srcdir)/src/portaudio/libportaudio.a \
	-lpthread \
	@M_LIBS@ \
	@LAME_LIBS@ \
	@OGG_LIBS@ \
	@VORBIS_LIBS@ \
	@GUI_LIBS@ \
	@SNDFILE_LIBS@ \
	@GLIB2_LIBS@

