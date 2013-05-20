pkginclude_HEADERS = \
	ArrayExactSize.h \
	ArraySingleItem.h \
	ArrayStringPrefix.h \
	ArrayStringSuffix.h \
	ArrayStringTrie.h \
	ArrayUtils.h \
	Constructors.h \
	Domain.h \
	Environment.h \
	FieldMinMax.h \
	FloatInterval.h \
	FloatUtils.h \
	IntegerBitfield.h \
	IntegerSet.h \
	IntegerInterval.h \
	IntegerUtils.h \
	InterpreterBasicBlock.h \
	InterpreterFunction.h \
	Interpreter.h \
	InterpreterIterator.h \
	InterpreterIteratorCallback.h \
	InterpreterModule.h \
	InterpreterOperationsCallback.h \
	Operations.h \
	OperationsCallback.h \
	Pointer.h \
	PointerTarget.h \
	PointerUtils.h \
	Prereq.h \
	ProductMessageField.h \
	ProductMessage.h \
	ProductVector.h \
	SharedDataPointer.h \
	SlotTracker.h \
	State.h \
	StateMap.h \
	Structure.h \
	Utils.h \
	VariableArguments.h \
	WideningDataInterface.h \
	WideningDataIterationCount.h \
	WideningInterface.h \
	WideningManager.h \
	WideningNumericalInfinity.h \
	WideningPointers.h

lib_LTLIBRARIES = libcanal.la
libcanal_la_SOURCES = \
	$(pkginclude_HEADERS) \
	ArrayExactSize.cpp \
	ArraySingleItem.cpp \
	ArrayStringPrefix.cpp \
	ArrayStringSuffix.cpp \
	ArrayStringTrie.cpp \
	ArrayUtils.cpp \
	Constructors.cpp \
	Domain.cpp \
	Environment.cpp \
	FieldMinMax.cpp \
	FloatInterval.cpp \
	FloatUtils.cpp \
	IntegerBitfield.cpp \
	IntegerSet.cpp \
	IntegerInterval.cpp \
	IntegerUtils.cpp \
	InterpreterBasicBlock.cpp \
	InterpreterFunction.cpp \
	Interpreter.cpp \
	InterpreterIterator.cpp \
	InterpreterModule.cpp \
	InterpreterOperationsCallback.cpp \
	Operations.cpp \
	Pointer.cpp \
	PointerTarget.cpp \
	PointerUtils.cpp \
	ProductMessage.cpp \
	ProductVector.cpp \
	SlotTracker.cpp \
	State.cpp \
	StateMap.cpp \
	Structure.cpp \
	Utils.cpp \
	VariableArguments.cpp \
	WideningDataIterationCount.cpp \
	WideningManager.cpp \
	WideningNumericalInfinity.cpp \
	WideningPointers.cpp

libcanal_la_CXXFLAGS = $(LLVM_CFLAGS)
libcanal_la_LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIBS) -version-info 0:0:0
