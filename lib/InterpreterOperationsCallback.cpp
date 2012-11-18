#include "InterpreterOperationsCallback.h"
#include "InterpreterFunction.h"
#include "InterpreterModule.h"
#include "Constructors.h"
#include "Utils.h"
#include "Domain.h"
#include <cstdio>

namespace Canal {
namespace Interpreter {

bool printMissing = true;

OperationsCallback::OperationsCallback(Module &module,
                                       Constructors &constructors)
    : mModule(module), mConstructors(constructors)
{
}

static Domain *
createTopReturnValue(const llvm::Function &function,
                     Constructors &constructors)
{
    const llvm::Type *type = function.getReturnType();
    if (type->isVoidTy())
        return NULL;

    Domain *result = constructors.create(*type);

    AccuracyDomain *accuracyValue = dynCast<AccuracyDomain*>(result);
    if (accuracyValue)
        accuracyValue->setTop();

    return result;
}

void
OperationsCallback::onFunctionCall(const llvm::Function &function,
                                   const State &callState,
                                   State &resultState,
                                   const llvm::Value &resultPlace)
{
    // Function not found.  Set the resultant value to the Top
    // value.
    if (function.isIntrinsic())
    {
        if (printMissing) {
            printf("Intrinsic function \"%s\" not available.\n",
               function.getName().str().c_str());
        }

        Domain *returnValue = createTopReturnValue(function, mConstructors);
        if (returnValue)
            resultState.addFunctionVariable(resultPlace, returnValue);

        return;
    }

    if (function.isDeclaration())
    {
        if (printMissing) {
            printf("External function \"%s\" not available.\n",
                function.getName().str().c_str());
        }

        Domain *returnValue = createTopReturnValue(function, mConstructors);
        if (returnValue)
            resultState.addFunctionVariable(resultPlace, returnValue);

        return;
    }

    Function *func = mModule.getFunction(function);
    CANAL_ASSERT_MSG(func, "Function not found in module!");

    // Extend the input so the function can be re-interpreted.
    func->getInputState().merge(callState);

    // Take the current function interpretation results and use them
    // as a result of the function call.
    resultState.mergeGlobal(func->getOutputState());
    resultState.mergeFunctionBlocks(func->getOutputState());
    if (func->getOutputState().getReturnedValue())
    {
        Domain *result = func->getOutputState().getReturnedValue()->clone();
        resultState.addFunctionVariable(resultPlace, result);
    }
}

} // namespace Interpreter
} // namespace Canal
