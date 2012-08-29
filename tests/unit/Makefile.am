## --------- ##
## Programs. ##
## --------- ##
noinst_PROGRAMS = InterpreterTest IntegerRangeTest PointerTest

InterpreterTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
InterpreterTest_SOURCES = InterpreterTest.cpp
InterpreterTest_LDADD = ../../lib/libcanal.la
InterpreterTest_LIBS = $(LLVM_LIBS)
InterpreterTest_LDFLAGS = $(LLVM_LDFLAGS)

IntegerRangeTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
IntegerRangeTest_SOURCES = IntegerRangeTest.cpp
IntegerRangeTest_LDADD = ../../lib/libcanal.la
IntegerRangeTest_LIBS = $(LLVM_LIBS)
IntegerRangeTest_LDFLAGS = $(LLVM_LDFLAGS)

PointerTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
PointerTest_SOURCES = PointerTest.cpp
PointerTest_LDADD = ../../lib/libcanal.la
PointerTest_LIBS = $(LLVM_LIBS)
PointerTest_LDFLAGS = $(LLVM_LDFLAGS)
