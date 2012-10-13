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
	Utils.cpp

canal_LDADD = ../lib/libcanal.la -lreadline -lhistory -lelf $(LLVM_LDFLAGS) $(LLVM_LIBS)
canal_CPPFLAGS = $(LLVM_CFLAGS) $(LIBEDIT_CFLAGS) -I.. -DDATADIR='"$(datadir)"'
