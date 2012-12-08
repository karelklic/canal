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

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/LoopInfo.h>
#if LLVM_VERSION_MAJOR > 2 || LLVM_VERSION_MINOR > 8
#include <llvm/InitializePasses.h>
#endif // LLVM_VERSION_MAJOR > 2 || LLVM_VERSION_MINOR > 8
#include <llvm/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TimeValue.h>
#include <llvm/ValueSymbolTable.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/ArgList.h>
#include <clang/Driver/Option.h>
#include <clang/Driver/Options.h>
#include <clang/Frontend/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#ifdef CANAL_NDEBUG_SWITCHED
#  undef CANAL_NDEBUG_SWITCHED
#  ifdef NDEBUG
#    undef NDEBUG
#  else
#    define NDEBUG
#  endif
#endif

#endif // CANAL_PREREQ_H
