add_library(canal SHARED
    APIntUtils.cpp
    ArrayExactSize.cpp
    ArraySingleItem.cpp
    ArrayStringPrefix.cpp
    Constructors.cpp
    Domain.cpp
    Environment.cpp
    FloatInterval.cpp
    FloatUtils.cpp
    IntegerBitfield.cpp
    IntegerSet.cpp
    IntegerInterval.cpp
    IntegerUtils.cpp
    InterpreterBasicBlock.cpp
    InterpreterFunction.cpp
    Interpreter.cpp
    InterpreterIterator.cpp
    InterpreterModule.cpp
    InterpreterOperationsCallback.cpp
    MemoryBlock.cpp
    MemoryBlockMap.cpp
    MemoryPointer.cpp
    MemoryState.cpp
    MemoryUtils.cpp
    MemoryVariableArguments.cpp
    MemoryVariableMap.cpp
    Operations.cpp
    ProductMessage.cpp
    ProductVector.cpp
    SlotTracker.cpp
    Structure.cpp
    Utils.cpp
    WideningDataIterationCount.cpp
    WideningManager.cpp
    WideningNumericalInfinity.cpp
    WideningPointers.cpp)

target_link_libraries(canal
    ${LLVM_MODULE_LIBS}
    ${LLVM_LDFLAGS})

install(TARGETS canal DESTINATION lib)
