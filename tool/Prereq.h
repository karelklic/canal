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
#include <llvm/LLVMContext.h>
#include <llvm/PassManager.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ValueSymbolTable.h>

#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR == 8
#  include <llvm/System/Host.h>
#  include <llvm/System/TimeValue.h>
#  include <llvm/Target/TargetSelect.h>
#elif LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR == 9
#  include <llvm/InitializePasses.h>
#  include <llvm/Support/Host.h>
#  include <llvm/Support/TimeValue.h>
#  include <llvm/Target/TargetSelect.h>
#elif LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR < 2
#  include <llvm/InitializePasses.h>
#  include <llvm/Support/Host.h>
#  include <llvm/Support/TimeValue.h>
#  include <llvm/Support/TargetSelect.h>
#else
#  include <llvm/InitializePasses.h>
#  include <llvm/Support/Host.h>
#  include <llvm/Support/TimeValue.h>
#  include <llvm/Support/TargetSelect.h>
#endif

#if HAVE_CLANG
#include <clang/Basic/Version.h>
#include <clang/Driver/Action.h>
#include <clang/Driver/Arg.h>
#include <clang/Driver/ArgList.h>
#include <clang/Driver/CC1Options.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Job.h>
#include <clang/Driver/Option.h>
#include <clang/Driver/Options.h>
#include <clang/Driver/OptTable.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#  if CLANG_VERSION_MAJOR < 3 || (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR < 2)
#    include <clang/Frontend/DiagnosticOptions.h>
#  else
#    include <clang/Basic/DiagnosticOptions.h>
#  endif
#endif

#ifdef CANAL_NDEBUG_SWITCHED
#  undef CANAL_NDEBUG_SWITCHED
#  ifdef NDEBUG
#    undef NDEBUG
#  else
#    define NDEBUG
#  endif
#endif

#endif // CANAL_PREREQ_H
