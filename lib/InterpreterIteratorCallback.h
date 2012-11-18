#ifndef LIBCANAL_INTERPRETER_ITERATOR_CALLBACK_H
#define LIBCANAL_INTERPRETER_ITERATOR_CALLBACK_H

#include "Prereq.h"

namespace Canal {
namespace Interpreter {

class BasicBlock;
class Function;

class IteratorCallback
{
public:
    virtual ~IteratorCallback() {}

    virtual void onFixpointReached() {}

    virtual void onModuleEnter() {}

    virtual void onModuleExit() {}

    virtual void onFunctionEnter(Function &function) {}

    virtual void onFunctionExit(Function &function) {}

    virtual void onBasicBlockEnter(BasicBlock &basicBlock) {}

    virtual void onBasicBlockExit(BasicBlock &basicBlock) {}

    virtual void onInstructionEnter(const llvm::Instruction &instruction) {}

    virtual void onInstructionExit(const llvm::Instruction &instruction) {}
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_ITERATOR_CALLBACK_H
