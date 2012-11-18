#ifndef LIBCANAL_PREREQ_H
#define LIBCANAL_PREREQ_H

#include "Config.h"

// Including the LLVM headers without the NDEBUG macro defined and then
// linking to LLVM dynamic libraries compiled with NDEBUG leads to
// runtime errors that are extremely difficult to analyze.
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

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/BasicBlock.h>
#include <llvm/Constant.h>
#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/GlobalVariable.h>
#include <llvm/Instruction.h>
#include <llvm/Instructions.h>
#include <llvm/Module.h>
#include <llvm/Support/CFG.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Type.h>
#include <llvm/Value.h>

#ifdef CANAL_NDEBUG_SWITCHED
#  undef CANAL_NDEBUG_SWITCHED
#  ifdef NDEBUG
#    undef NDEBUG
#  else
#    define NDEBUG
#  endif
#endif

#endif // LIBCANAL_PREREQ_H
