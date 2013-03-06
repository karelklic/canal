SUBDIRS = lib tool tests docs

# Automatic update of libtool.
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = canal.pc
DISTCLEANFILES = $(pkgconfig_DATA)
EXTRA_DIST = canal.pc.in

UPLOAD_URI = kklic@fedorahosted.org:canal
.PHONY: upload
upload:
	make dist-gzip
	scp $(distdir).tar.gz $(UPLOAD_URI)
	make dist-zip
	scp $(distdir).zip $(UPLOAD_URI)
	make dist-xz
	scp $(distdir).tar.xz $(UPLOAD_URI)

RPM_DIRS = --define "_sourcedir `pwd`" \
           --define "_rpmdir `pwd`" \
           --define "_specdir `pwd`" \
           --define "_builddir `pwd`" \
           --define "_srcrpmdir `pwd`"

EXTRA_DIST += canal.spec.in
rpm: dist-xz canal.spec
	rpmbuild $(RPM_DIRS) -ba canal.spec
srpm: dist-xz canal.spec
	rpmbuild $(RPM_DIRS) -bs canal.spec
