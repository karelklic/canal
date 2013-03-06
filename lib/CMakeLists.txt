add_library(canal SHARED
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
