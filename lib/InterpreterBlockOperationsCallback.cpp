#include "InterpreterBlockOperationsCallback.h"
#include "Utils.h"

namespace Canal {
namespace InterpreterBlock {

OperationsCallback::OperationsCallback(Module &module)
    : mModule(module)
{
}

Domain *
OperationsCallback::onFunctionCall(const llvm::Function &function,
                                   const std::vector<Domain*> &arguments)
{
    CANAL_NOT_IMPLEMENTED();

/*
    // TODO: Handle some intristic functions.  Some of them can be
    // safely ignored.
    if (!function || function->isIntrinsic() || function->isDeclaration())
    {
        // Function not found.  Set the resultant value to the Top
        // value.
        printf("Function \"%s\" not available.\n", function->getName().data());

        // TODO: Set memory accessed by non-static globals to
        // the Top value.

        // Create result TOP value of required type.
        const llvm::Type *type = instruction.getType();
        Domain *returnedValue = mConstructors.create(*instruction.getType());

        // If the function returns nothing (void), we are finished.
        if (!returnedValue)
            return;

        AccuracyDomain *accuracyValue = dynCast<AccuracyDomain*>(returnedValue);
        if (accuracyValue)
            accuracyValue->setTop();

        state.addFunctionVariable(instruction, returnedValue);
        return;
    }

    State initialState(state);
    initialState.clearFunctionLevel();
    llvm::Function::ArgumentListType::const_iterator it =
        function->getArgumentList().begin();

    for (int i = 0; i < instruction.getNumArgOperands(); ++i, ++it)
    {
        llvm::Value *operand = instruction.getArgOperand(i);

        llvm::OwningPtr<Domain> constant;
        Domain *value = variableOrConstant(*operand, state, constant);
        if (!value)
            return;

        initialState.addFunctionVariable(*it, value->clone());
    }

    stack.addFrame(*function, initialState);
*/
}

} // namespace InterpreterBlock
} // namespace Canal
