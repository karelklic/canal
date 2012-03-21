lib_LTLIBRARIES = libcanal.la
libcanal_la_SOURCES = \
	AbstractArray.h \
	AbstractInteger.h \
	AbstractPointer.h \
	AbstractReal.h \
	AbstractStructure.h \
	AbstractValue.h \
	OperationalInterpreter.cpp \
	OperationalInterpreter.h \
	OperationalMachine.cpp \
	OperationalMachine.h \
	OperationalState.cpp \
	OperationalState.h \
	SELinuxModulePass.cpp

libcanal_la_CXXFLAGS = $(LLVM_CFLAGS)
libcanal_la_LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) -version-info 0:0:0
