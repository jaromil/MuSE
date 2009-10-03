INCLUDES = -I$(top_srcdir)/src -I/sw/include -Ilibcdk

EXTRA_DIST = coltable.c muse_console.cpp testgui.sh

noinst_LIBRARIES = librubik.a

noinst_HEADERS = about.h connectbox.h debug.h encdata.h encsetbox.h entry.h \
fselect.h itemlist.h label.h menu.h muse_console.h muse_tui.h ncurses_gui.h \
play_infobox.h screen.h scroll.h single_channel.h slider.h streamsetbox.h

librubik_a_SOURCES = \
	entry.cpp \
	fselect.cpp \
	label.cpp \
	muse_tui.cpp \
	play_infobox.cpp \
	screen.cpp \
	scroll.cpp \
	single_channel.cpp \
	slider.cpp \
	about.cpp \
	connectbox.cpp \
	encsetbox.cpp \
	streamsetbox.cpp \
	itemlist.cpp \
	menu.cpp \
	ncurses_gui.cpp

SUBDIRS = libcdk
