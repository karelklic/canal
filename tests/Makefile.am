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

EXTRA_DIST = \
	0-numeric.c \
	1-pointer.c \
	2-string.c

## --------- ##
## Programs. ##
## --------- ##
noinst_PROGRAMS = OperationsTest IntegerIntervalTest PointerTest ICMPTest MergeTest

OperationsTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
OperationsTest_SOURCES = OperationsTest.cpp
OperationsTest_LDADD = ../lib/libcanal.la
OperationsTest_LIBS = $(LLVM_LIBS)
OperationsTest_LDFLAGS = $(LLVM_LDFLAGS)

IntegerIntervalTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
IntegerIntervalTest_SOURCES = IntegerIntervalTest.cpp
IntegerIntervalTest_LDADD = ../lib/libcanal.la
IntegerIntervalTest_LIBS = $(LLVM_LIBS)
IntegerIntervalTest_LDFLAGS = $(LLVM_LDFLAGS)

PointerTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
PointerTest_SOURCES = PointerTest.cpp
PointerTest_LDADD = ../lib/libcanal.la
PointerTest_LIBS = $(LLVM_LIBS)
PointerTest_LDFLAGS = $(LLVM_LDFLAGS)

ICMPTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
ICMPTest_SOURCES = ICMPTest.cpp
ICMPTest_LDADD = ../lib/libcanal.la
ICMPTest_LIBS = $(LLVM_LIBS)
ICMPTest_LDFLAGS = $(LLVM_LDFLAGS)

MergeTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
MergeTest_SOURCES = MergeTest.cpp
MergeTest_LDADD = ../lib/libcanal.la
MergeTest_LIBS = $(LLVM_LIBS)
MergeTest_LDFLAGS = $(LLVM_LDFLAGS)
