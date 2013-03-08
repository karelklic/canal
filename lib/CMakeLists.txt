add_library(canal SHARED
    ArrayExactSize.cpp
    ArraySingleItem.cpp
    ArrayStringPrefix.cpp
<<<<<<< HEAD
    ArrayUtils.cpp
=======
    ArrayStringTrie.cpp
>>>>>>> e4e9cfc27d255e8d5a2126579041e4f9a69add82
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
    Operations.cpp
    Pointer.cpp
    PointerTarget.cpp
    PointerUtils.cpp
    ProductMessage.cpp
    ProductVector.cpp
    SlotTracker.cpp
    State.cpp
    StateMap.cpp
    Structure.cpp
    Utils.cpp
    VariableArguments.cpp
    WideningDataIterationCount.cpp
    WideningManager.cpp
    WideningNumericalInfinity.cpp
    WideningPointers.cpp)

target_link_libraries(canal
    ${LLVM_MODULE_LIBS}
    ${LLVM_LDFLAGS})

install(TARGETS canal DESTINATION lib)
