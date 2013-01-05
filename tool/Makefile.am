bin_PROGRAMS = canal
canal_SOURCES = \
	Canal.cpp \
	Command.h \
	Command.cpp \
	CommandBreak.h \
	CommandBreak.cpp \
	CommandCd.h \
	CommandCd.cpp \
	CommandContinue.h \
	CommandContinue.cpp \
	CommandDump.h \
	CommandDump.cpp \
	CommandFile.h \
	CommandFile.cpp \
	CommandFinish.h \
	CommandFinish.cpp \
	CommandHelp.h \
	CommandHelp.cpp \
	CommandInfo.h \
	CommandInfo.cpp \
	CommandPrint.h \
	CommandPrint.cpp \
	CommandPwd.h \
	CommandPwd.cpp \
	CommandQuit.h \
	CommandQuit.cpp \
	CommandRun.h \
	CommandRun.cpp \
	CommandSet.h \
	CommandSet.cpp \
	CommandShell.h \
	CommandShell.cpp \
	CommandShow.h \
	CommandShow.cpp \
	CommandStart.h \
	CommandStart.cpp \
	CommandStep.h \
	CommandStep.cpp \
	Commands.h \
	Commands.cpp \
	IteratorCallback.h \
	IteratorCallback.cpp \
	State.h \
	State.cpp \
	Utils.h \
	Utils.cpp \
	WrapperGcc.h \
	WrapperGcc.cpp

canal_LDADD = $(top_builddir)/lib/libcanal.la -lreadline -lhistory $(LLVM_LDFLAGS) $(CLANG_LIBS) $(LLVM_LIBS)
canal_CPPFLAGS = $(LLVM_CFLAGS) $(LIBEDIT_CFLAGS) -I$(top_srcdir) -DDATADIR='"$(datadir)"'

install-exec-hook:
	mkdir -p $(DESTDIR)$(datadir)/canal/wrappers && \
	cd $(DESTDIR)$(datadir)/canal/wrappers && \
	$(LN_S)	-f $(DESTDIR)$(bindir)/canal$(EXEEXT) gcc$(EXEEXT) && \
	$(LN_S)	-f $(DESTDIR)$(bindir)/canal$(EXEEXT) g++$(EXEEXT) && \
	$(LN_S)	-f $(DESTDIR)$(bindir)/canal$(EXEEXT) ld$(EXEEXT)
