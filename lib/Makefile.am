lib_LTLIBRARIES = libcanal.la
libcanal_la_SOURCES = \
	Array.h \
	Array.cpp \
	ArrayExactLimitedSize.h \
	ArrayExactLimitedSize.cpp \
	ArraySingleItem.h \
	ArraySingleItem.cpp \
	Constant.h \
	Constant.cpp \
	Float.h \
	Float.cpp \
	Integer.h \
	Integer.cpp \
	IntegerBits.h \
	IntegerBits.cpp \
	IntegerEnumeration.h \
	IntegerEnumeration.cpp \
	IntegerRange.h \
	IntegerRange.cpp \
	Interpreter.h \
	Interpreter.cpp \
	Pointer.h \
	Pointer.cpp \
	SELinuxModulePass.cpp \
	SlotTracker.h \
	SlotTracker.cpp \
	Stack.h \
	Stack.cpp \
	State.h \
	State.cpp \
	Structure.h \
	Structure.cpp \
	Utils.h \
	Utils.cpp \
	Value.h \
	Value.cpp

libcanal_la_CXXFLAGS = $(LLVM_CFLAGS)
libcanal_la_LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) -version-info 0:0:0
