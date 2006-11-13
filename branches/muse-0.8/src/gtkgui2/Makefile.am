localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

INCLUDES = -I. -I../.. -I$(top_srcdir)/src @GTK2_CFLAGS@

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
	radiosched.cpp
