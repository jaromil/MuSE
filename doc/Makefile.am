EXTRA_DIST = muse.doxygen.in

installdest = $(DESTDIR)$(docdir)/
htmlbuild = html

$(htmlbuild)/index.html: muse.doxygen
	$(DOXYGEN) muse.doxygen

noinst_DATA = $(htmlbuild)/index.html

install-data-local:
	$(INSTALL) -d $(installdest)
	for d in `find $(htmlbuild) -type d`; do $(INSTALL) -m755 -d  $(installdest)/$$d; done
	for f in `find $(htmlbuild) -type f`; do $(INSTALL) -m644 $$f $(installdest)/$$f; done

uninstall-local:
	rm -rf $(installdest)/$(htmlbuild)/

clean-local:
	rm -rf html latex rtf man
