#include "InterpreterOperationsCallback.h"
#include "InterpreterFunction.h"
#include "InterpreterModule.h"
#include "Constructors.h"
#include "Utils.h"
#include "Domain.h"

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

    if (type->isPointerTy())
    {
        // We return empty pointer for now.  This is unsafe.
        return constructors.create(*type);
    }
    else
    {
        Domain *result = constructors.create(*type);
        result->setTop();
        return result;
    }
}

void
OperationsCallback::onFunctionCall(const llvm::Function &function,
                                   const Memory::State &callState,
                                   Memory::State &resultState,
                                   const llvm::Value &resultPlace)
{
    if (function.getName() == "strcat")
    {
        onFunctionCallStrcat(function, callState, resultState, resultPlace);
        return;
    }

    // Function not found.  Set the resultant value to the Top
    // value.
    if (function.isIntrinsic())
    {
        if (printMissing)
        {
            llvm::outs() << "Intrinsic function \""
                         << function.getName()
                         << "\" not available.\n";
        }

        Domain *returnValue = createTopReturnValue(function, mConstructors);
        if (returnValue)
            resultState.addFunctionVariable(resultPlace, returnValue);

        return;
    }

    if (function.isDeclaration())
    {
        if (printMissing)
        {
            llvm::outs() << "External function \""
                         << function.getName()
                         << "\" not available.\n";
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

void
OperationsCallback::onFunctionCallStrcat(const llvm::Function &function,
                                         const Memory::State &callState,
                                         Memory::State &resultState,
                                         const llvm::Value &resultPlace)
{
    CANAL_ASSERT(function.getArgumentList().size() == 2);
    //const llvm::Argument &destArgument = *function.getArgumentList().begin();
    //const llvm::Argument &srcArgument = *(++function.getArgumentList().begin());
    //const Domain *destinationPointer = callState.findVariable(destArgument);
    //const Domain *sourcePointer = callState.findVariable(srcArgument);

    CANAL_NOT_IMPLEMENTED();

    //resultState.addFunctionVariable(resultPlace, destinationPointer->clone());
}

} // namespace Interpreter
} // namespace Canal
