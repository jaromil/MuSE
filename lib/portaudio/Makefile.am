EXTRA_DIST = LICENSE

AUTOMAKE_OPTIONS = foreign

noinst_HEADERS = pa_host.h pa_trace.h pa_unix.h portaudio.h ringbuffer.h

noinst_LIBRARIES = libportaudio.a

libportaudio_a_SOURCES = \
	pa_convert.c \
	pa_lib.c \
	pa_trace.c \
	pa_unix.c \
	pa_unix_oss.c \
	pa_unix_solaris.c \
	pa_mac.c \
	pa_mac_core.c \
	ringbuffer.c
