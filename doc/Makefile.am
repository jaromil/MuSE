EXTRA_DIST = muse.doxygen.in

installdest = $(DESTDIR)$(docdir)/
htmlbuild = html

doxygen-stamp: muse.doxygen
	$(DOXYGEN) muse.doxygen
	touch doxygen-stamp

noinst_DATA = doxygen-stamp

install-data-local:
	$(INSTALL) -d $(installdest)
	for d in `find $(htmlbuild) -type d`; do $(INSTALL) -m755 -d  $(installdest)/$$d; done
	for f in `find $(htmlbuild) -type f`; do $(INSTALL) -m644 $$f $(installdest)/$$f; done

uninstall-local:
	rm -rf $(installdest)/$(htmlbuild)/

clean-local:
	rm -rf doxygen-stamp
	rm -rf html latex rtf man
