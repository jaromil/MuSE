if BUILD_DOC
DOCSDIR = doc
endif

SUBDIRS = lib src $(DOCSDIR)

dist_docs_DATA = README COPYING AUTHORS NEWS KNOWN-BUGS ChangeLog TODO USAGE
docsdir = ${prefix}/doc/MuSE

dist_man_MANS = muse.1

ACLOCAL_AMFLAGS = -I m4

# FIXME po should be properly gettexited
EXTRA_DIST = autogen.sh po utils
