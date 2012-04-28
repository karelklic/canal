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
	State.h \
	State.cpp \
	Utils.h \
	Utils.cpp

canal_LDADD = ../lib/libcanal.la -lreadline -lhistory $(LLVM_LDFLAGS) $(LLVM_LIBS)
canal_CPPFLAGS = $(LLVM_CFLAGS) $(LIBEDIT_CFLAGS)
