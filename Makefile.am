ACLOCAL_AMFLAGS = -I m4

if BUILD_DOCUMENTATION
  MAYBE_DOCUMENTATION = docs
endif

SUBDIRS = lib tool tests $(MAYBE_DOCUMENTATION)

EXTRA_DIST = canal.spec.in

# Automatic update of libtool.
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

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

rpm: dist-xz canal.spec
	rpmbuild $(RPM_DIRS) -ba canal.spec
srpm: dist-xz canal.spec
	rpmbuild $(RPM_DIRS) -bs canal.spec
