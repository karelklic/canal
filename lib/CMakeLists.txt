add_library(canal SHARED APIntUtils.cpp
ArrayExactSize.cpp
ArrayInterface.cpp
ArraySingleItem.cpp
Constant.cpp
Domain.cpp
Environment.cpp
FloatInterval.cpp
IntegerBitfield.cpp
IntegerContainer.cpp
IntegerEnumeration.cpp
IntegerInterval.cpp
Operations.cpp
Pointer.cpp
PointerTarget.cpp
SELinuxModulePass.cpp
SlotTracker.cpp
Stack.cpp
State.cpp
StringUtils.cpp
Structure.cpp
Utils.cpp)

target_link_libraries(canal "${LLVM_MODULE_LIBS} ${LLVM_LDFLAGS}")
