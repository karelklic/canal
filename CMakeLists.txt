cmake_minimum_required(VERSION 2.8)
# A convenience variable:
include("llvm-root.cmake")
# A bit of a sanity check:
if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
endif()
# We incorporate the CMake features provided by LLVM:
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/share/llvm/cmake")
include(LLVMConfig)
include(AddLLVM)
# Now set the header and library paths:
include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )
# Let's suppose we want to build a JIT compiler with support for
# binary code (no interpreter):
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES jit native)
# Finally, we link the LLVM libraries to our executable:
#target_link_libraries(mycompiler ${REQ_LLVM_LIBRARIES})
add_subdirectory("lib/")
