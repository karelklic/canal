## ------------ ##
## package.m4.  ##
## ------------ ##

package.m4: Makefile.in
	{ \
             echo '# Signature of the current package.' && \
             echo 'm4_define([AT_PACKAGE_NAME],' && \
             echo '  [@PACKAGE_NAME@])' && \
             echo 'm4_define([AT_PACKAGE_TARNAME],' && \
             echo '  [@PACKAGE_TARNAME@])' && \
             echo 'm4_define([AT_PACKAGE_VERSION],' && \
             echo '  [@PACKAGE_VERSION@])' && \
             echo 'm4_define([AT_PACKAGE_STRING],' && \
             echo '  [@PACKAGE_STRING@])' && \
             echo 'm4_define([AT_PACKAGE_BUGREPORT],' && \
             echo '  [@PACKAGE_BUGREPORT@])'; \
             echo 'm4_define([AT_PACKAGE_URL],' && \
             echo '  [@PACKAGE_URL@])'; \
	} >'package.m4'
EXTRA_DIST = package.m4

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT =	\
  local.at \
  testsuite.at

EXTRA_DIST += $(TESTSUITE_AT)
TESTSUITE = $(srcdir)/testsuite
EXTRA_DIST += $(TESTSUITE)
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE)
check_DATA = atconfig atlocal $(TESTSUITE)
DISTCLEANFILES = atconfig
EXTRA_DIST += atlocal.in

atconfig: $(top_builddir)/config.status
	(cd ${top_builddir} && ./config.status ${subdir}/atconfig)

check-local: $(check_DATA)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: $(check_DATA)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' $(TESTSUITEFLAGS) ||:

clean-local:
	test ! -f '$(TESTSUITE)' || $(SHELL) '$(TESTSUITE)' --clean

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(TESTSUITE_AT) $(srcdir)/package.m4
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

## ----------- ##
## Test files. ##
## ----------- ##
EXTRA_DIST += \
	0-numeric.c \
	1-pointer.c \
	2-string.c

## --------- ##
## Programs. ##
## --------- ##
AM_DEFAULT_SOURCE_EXT = .cpp
AM_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I$(top_srcdir)
LDADD = $(top_builddir)/lib/libcanal.la
AM_LIBS = $(LLVM_LIBS)
AM_LDFLAGS = $(LLVM_LDFLAGS)

noinst_PROGRAMS = \
	ArrayExactSizeTest \
	ArraySingleItemTest \
	FloatIntervalTest \
	IntegerBitfieldTest \
	IntegerSetTest \
	IntegerIntervalTest \
	OperationsTest \
	PointerTest
