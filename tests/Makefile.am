## --------- ##
## Programs. ##
## --------- ##
#noinst_PROGRAMS = InterpreterTest PointerTest

#InterpreterTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
#InterpreterTest_SOURCES = InterpreterTest.cpp
#InterpreterTest_LDADD = ../lib/libcanal.la
#InterpreterTest_LIBS = $(LLVM_LIBS)
#InterpreterTest_LDFLAGS = $(LLVM_LDFLAGS)

#PointerTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I..
#PointerTest_SOURCES = PointerTest.cpp
#PointerTest_LDADD = ../lib/libcanal.la
#PointerTest_LIBS = $(LLVM_LIBS)
#PointerTest_LDFLAGS = $(LLVM_LDFLAGS)

## ----------- ##
## Test files. ##
## ----------- ##

EXTRA_DIST = numeric0.c

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
EXTRA_DIST += package.m4

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT =	\
  local.at 	\
  testsuite.at	\
  Value.at \
  Interpreter.at

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
