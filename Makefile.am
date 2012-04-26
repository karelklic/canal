ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib tests

bin_PROGRAMS = canal
canal_SOURCES = \
	Canal.cpp \
	Command.h \
	Command.cpp \
	CommandBacktrace.h \
	CommandBacktrace.cpp \
	CommandBreak.h \
	CommandBreak.cpp \
	CommandCd.h \
	CommandCd.cpp \
	CommandContinue.h \
	CommandContinue.cpp \
	CommandFile.h \
	CommandFile.cpp \
	CommandFinish.h \
	CommandFinish.cpp \
	CommandHelp.h \
	CommandHelp.cpp \
	CommandInfo.h \
	CommandInfo.cpp \
	CommandNext.h \
	CommandNext.cpp \
	CommandPrint.h \
	CommandPrint.cpp \
	CommandPwd.h \
	CommandPwd.cpp \
	CommandQuit.h \
	CommandQuit.cpp \
	CommandRun.h \
	CommandRun.cpp \
	CommandShow.h \
	CommandShow.cpp \
	CommandStart.h \
	CommandStart.cpp \
	CommandStep.h \
	CommandStep.cpp \
	Commands.h \
	Commands.cpp \
	State.h \
	State.cpp \
	Utils.h \
	Utils.cpp

canal_LDADD = lib/libcanal.la -lreadline -lhistory $(LLVM_LDFLAGS) $(LLVM_LIBS)
canal_CPPFLAGS = $(LLVM_CFLAGS) $(LIBEDIT_CFLAGS)

EXTRA_DIST = canal.spec.in

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
