localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

INCLUDES = -I. -I../.. -I$(top_srcdir)/src @GTK2_CFLAGS@ -I/sw/include

EXTRA_DIST = xpm2

noinst_HEADERS = about.h docking.h encoder.h filedump.h gchan_events.h gchan.h \
	gen.h gradiosched.h gtk2_gui.h gtkdial.h ice.h listpack.h musegtk.h utils.h \
	vumeters.h xmlprofile.h

noinst_LIBRARIES = libnightolo2.a

libnightolo2_a_SOURCES =	\
	docking.cpp	\
	gchan.cpp	\
	gchan_events.cpp	\
	ice.cpp	\
	encoder.cpp	\
	filedump.cpp	\
	musegtk.cpp	\
	listpack.cpp	\
	gtkdial.cpp \
	utils.cpp	\
	vumeters.cpp \
	about.cpp \
	gtk2_gui.cpp \
	xmlprofile.cpp \
	gradiosched.cpp
