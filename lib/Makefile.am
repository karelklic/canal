lib_LTLIBRARIES = libcanal.la
libcanal_la_SOURCES = \
	Array.h \
	Array.cpp \
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
	Interpreter.h \
	Interpreter.cpp \
	Pointer.h \
	Pointer.cpp \
	SELinuxModulePass.cpp \
	State.h \
	State.cpp \
	Structure.h \
	Utils.h \
	Value.h \
	Value.cpp

libcanal_la_CXXFLAGS = $(LLVM_CFLAGS)
libcanal_la_LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) -version-info 0:0:0
