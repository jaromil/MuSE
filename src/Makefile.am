SUBDIRS = @GUI_DIRS@

INCLUDES = -I/usr/X11R6/include -Incursesgui @GLIB2_CFLAGS@ \
		@DARWIN_CFLAGS@ @PORTAUDIO_CFLAGS@ @SHOUT_CFLAGS@ \
		@VORBIS_CFLAGS@ @OGG_CFLAGS@ @SAMPLERATE_CFLAGS@ \
		@LIBMPEG_CFLAGS@

CFLAGS = @GUI_FLAGS@ @JACK_CFLAGS@ @OGG_CFLAGS@ @VORBIS_CFLAGS@

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
    dev_sound.cpp

muse_LDADD = \
	-lpthread \
	@SAMPLERATE_LIBS@ \
	@M_LIBS@ \
	@LAME_LIBS@ \
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
