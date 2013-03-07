#include "InterpreterOperationsCallback.h"
#include "InterpreterFunction.h"
#include "InterpreterModule.h"
#include "Constructors.h"
#include "Utils.h"
#include "Domain.h"
#include "Pointer.h"
#include "ArrayUtils.h"

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
                                   const State &callState,
                                   State &resultState,
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
                                         const State &callState,
                                         State &resultState,
                                         const llvm::Value &resultPlace)
{
    CANAL_ASSERT(function.getArgumentList().size() == 2);
    const llvm::Argument &destArgument = *function.getArgumentList().begin();
    const llvm::Argument &srcArgument = *(++function.getArgumentList().begin());
    const Domain *destDomain = callState.findVariable(destArgument);
    const Domain *srcDomain = callState.findVariable(srcArgument);
    const Pointer::Pointer *destPointer = checkedCast<Pointer::Pointer>(destDomain);
    const Pointer::Pointer *srcPointer = checkedCast<Pointer::Pointer>(srcDomain);
    CANAL_ASSERT_MSG(destPointer->isConstant(), "Not implemented for variable pointers");
    CANAL_ASSERT_MSG(srcPointer->isConstant(), "Not implemented for variable pointers");
    CANAL_ASSERT(destPointer->mTargets.begin()->second->mType == Pointer::Target::Block);
    CANAL_ASSERT(srcPointer->mTargets.begin()->second->mType == Pointer::Target::Block);
    const llvm::Value &destTarget = *destPointer->mTargets.begin()->second->mTarget;
    const llvm::Value &srcTarget = *srcPointer->mTargets.begin()->second->mTarget;
    const Domain *destArray = callState.findBlock(destTarget);
    const Domain *srcArray = callState.findBlock(srcTarget);
    CANAL_ASSERT(destArray && srcArray);
    Domain *newArray = destArray->clone();

    Array::Utils::strcat(*newArray, *srcArray);

    // Works just on function blocks for now.
    resultState.addFunctionBlock(destTarget, newArray);
    resultState.addFunctionVariable(resultPlace, destPointer->clone());
}

} // namespace Interpreter
} // namespace Canal
