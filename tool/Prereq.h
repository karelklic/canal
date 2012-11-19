#ifndef CANAL_PREREQ_H
#define CANAL_PREREQ_H

#include "lib/Prereq.h"

#ifdef LLVM_WITH_NDEBUG
#  ifndef NDEBUG
#    define NDEBUG
#    define CANAL_NDEBUG_SWITCHED
#  endif
#else
#  ifdef NDEBUG
#    undef NDEBUG
#    define CANAL_NDEBUG_SWITCHED
#  endif
#endif

#include <llvm/ADT/StringRef.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/LoopInfo.h>
#if LLVM_VERSION_MAJOR > 2 || LLVM_VERSION_MINOR > 8
#include <llvm/InitializePasses.h>
#endif // LLVM_VERSION_MAJOR > 2 || LLVM_VERSION_MINOR > 8
#include <llvm/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ValueSymbolTable.h>

#ifdef CANAL_NDEBUG_SWITCHED
#  undef CANAL_NDEBUG_SWITCHED
#  ifdef NDEBUG
#    undef NDEBUG
#  else
#    define NDEBUG
#  endif
#endif

#endif // CANAL_PREREQ_H
