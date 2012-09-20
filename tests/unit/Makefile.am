## --------- ##
## Programs. ##
## --------- ##
noinst_PROGRAMS = OperationsTest IntegerIntervalTest PointerTest

OperationsTest_CXXFLAGS = -Wall $(LLVM_CFLAGS) -I../..
OperationsTest_SOURCES = OperationsTest.cpp
OperationsTest_LDADD = ../../lib/libcanal.la
OperationsTest_LIBS = $(LLVM_LIBS)
OperationsTest_LDFLAGS = $(LLVM_LDFLAGS)

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
