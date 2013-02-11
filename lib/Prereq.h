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
#include <llvm/ADT/STLExtras.h>
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
#include <llvm/Type.h>
#include <llvm/Value.h>
#if LLVM_VERSION_MAJOR > 2 && LLVM_VERSION_MINOR > 1
#  include <llvm/DataLayout.h>
#else
#  include <llvm/Target/TargetData.h>
#endif

#ifdef CANAL_NDEBUG_SWITCHED
#  undef CANAL_NDEBUG_SWITCHED
#  ifdef NDEBUG
#    undef NDEBUG
#  else
#    define NDEBUG
#  endif
#endif

// Class declarations.
namespace Canal {

    class Constructors;
    class Domain;
    class Environment;
    class Operations;
    class OperationsCallback;
    class SlotTracker;

    namespace Integer {
        class Bitfield;
        class Interval;
        class Set;
    } // namespace Integer

    namespace Interpreter {
        class BasicBlock;
        class Function;
        class IteratorCallback;
        class Module;
    } // namespace Interpreter

    namespace Memory {
        class State;
    } // namespace Memory

    namespace Product {
        class Message;
    } // namespace Product

    namespace Widening {
        class DataInterface;
        class Interface;
        class Manager;
    } // namespace Widening

} // namespace Canal

#endif // LIBCANAL_PREREQ_H
