SUBDIRS = resample shout libmpeg @GUI_DIRS@

INCLUDES = -I/usr/X11R6/include @GUI_FLAGS@ -I$(top_srcdir)/src/ncursesgui

bin_PROGRAMS = muse

muse_SOURCES =  \
    muse.cpp        \
    jmixer.cpp      \
    audioproc.cpp       \
    inchannels.cpp      \
    in_oggvorbis.cpp    \
    in_mpeg.cpp     \
    outchannels.cpp     \
    out_vorbis.cpp	\
    out_lame.cpp	\
    pipe.cpp        \
    shouter.cpp     \
    linklist.cpp        \
    playlist.cpp        \
    jutils.cpp      \
    gui.cpp

muse_DEPENDENCIES = @GUI_DEPS@

muse_LDADD = $(top_srcdir)/src/resample/libresample.a $(top_srcdir)/src/shout/libshout.a $(top_srcdir)/src/libmpeg/libmpeg.a -lpthread @M_LIBS@ @LAME_LIBS@ @OGG_LIBS@ @VORBIS_LIBS@ @GUI_LIBS@

