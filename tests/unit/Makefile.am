## --------- ##
## Programs. ##
## --------- ##
noinst_PROGRAMS = InterpreterTest IntegerIntervalTest PointerTest

InterpreterTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
InterpreterTest_SOURCES = InterpreterTest.cpp
InterpreterTest_LDADD = ../../lib/libcanal.la
InterpreterTest_LIBS = $(LLVM_LIBS)
InterpreterTest_LDFLAGS = $(LLVM_LDFLAGS)

IntegerIntervalTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
IntegerIntervalTest_SOURCES = IntegerIntervalTest.cpp
IntegerIntervalTest_LDADD = ../../lib/libcanal.la
IntegerIntervalTest_LIBS = $(LLVM_LIBS)
IntegerIntervalTest_LDFLAGS = $(LLVM_LDFLAGS)

PointerTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
PointerTest_SOURCES = PointerTest.cpp
PointerTest_LDADD = ../../lib/libcanal.la
PointerTest_LIBS = $(LLVM_LIBS)
PointerTest_LDFLAGS = $(LLVM_LDFLAGS)
