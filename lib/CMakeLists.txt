add_library(canal SHARED
APIntUtils.cpp
ArrayExactSize.cpp
ArrayInterface.cpp
ArraySingleItem.cpp
Constructors.cpp
Domain.cpp
Environment.cpp
FloatInterval.cpp
IntegerBitfield.cpp
IntegerContainer.cpp
IntegerEnumeration.cpp
IntegerInterval.cpp
InterpreterBlockBasicBlock.cpp
InterpreterBlockFunction.cpp
InterpreterBlock.cpp
InterpreterBlockIterator.cpp
InterpreterBlockModule.cpp
InterpreterBlockOperationsCallback.cpp
Operations.cpp
Pointer.cpp
PointerTarget.cpp
SlotTracker.cpp
State.cpp
StateMap.cpp
StringUtils.cpp
Structure.cpp
Utils.cpp
VariableArguments.cpp
WideningDataIterationCount.cpp
WideningManager.cpp
WideningNumericalInfinity.cpp)

target_link_libraries(canal "${LLVM_MODULE_LIBS} ${LLVM_LDFLAGS}")
