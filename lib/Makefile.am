lib_LTLIBRARIES = libcanal.la
libcanal_la_SOURCES = \
	Array.h \
	Constant.h \
	Constant.cpp \
	IntegerBits.h \
	Integer.cpp \
	IntegerEnumeration.h \
	Integer.h \
	IntegerRange.h \
	Interpreter.cpp \
	Interpreter.h \
	Pointer.cpp \
	Pointer.h \
	Real.h \
	SELinuxModulePass.cpp \
	State.cpp \
	State.h \
	Structure.h \
	Utils.h \
	Value.cpp \
	Value.h

libcanal_la_CXXFLAGS = $(LLVM_CFLAGS)
libcanal_la_LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) -version-info 0:0:0
