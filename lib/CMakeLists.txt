add_library(canal SHARED APIntUtils.cpp
ArrayExactSize.cpp
ArrayInterface.cpp
ArraySingleItem.cpp
Constant.cpp
Domain.cpp
Environment.cpp
FloatRange.cpp
IntegerBits.cpp
IntegerContainer.cpp
IntegerEnumeration.cpp
IntegerRange.cpp
Interpreter.cpp
Pointer.cpp
PointerTarget.cpp
SELinuxModulePass.cpp
SlotTracker.cpp
Stack.cpp
State.cpp
StringUtils.cpp
Structure.cpp
Utils.cpp)
target_link_libraries(canal ${REQ_LLVM_LIBRARIES})
