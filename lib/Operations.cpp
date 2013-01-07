#include "Operations.h"
#include "ArrayExactSize.h"
#include "ArraySingleItem.h"
#include "Constructors.h"
#include "Environment.h"
#include "FloatInterval.h"
#include "IntegerBitfield.h"
#include "IntegerContainer.h"
#include "IntegerUtils.h"
#include "OperationsCallback.h"
#include "Pointer.h"
#include "PointerUtils.h"
#include "Structure.h"
#include "Utils.h"
#include "Domain.h"
#include "State.h"
#include "Memory.h"
#include <map>
#include <cassert>
#include <cstdio>

namespace Canal {

Operations::Operations(const Environment &environment,
                       const Constructors &constructors,
                       OperationsCallback &callback)
    : mEnvironment(environment),
      mConstructors(constructors),
      mCallback(callback)
{
}

void
Operations::interpretInstruction(const llvm::Instruction &instruction,
                                 State &state)
{
    if (llvm::isa<llvm::CallInst>(instruction))
        call((const llvm::CallInst&)instruction, state);
    else if (llvm::isa<llvm::ICmpInst>(instruction))
        icmp((const llvm::ICmpInst&)instruction, state);
    else if (llvm::isa<llvm::FCmpInst>(instruction))
        fcmp((const llvm::FCmpInst&)instruction, state);
    else if (llvm::isa<llvm::ExtractElementInst>(instruction))
        extractelement((const llvm::ExtractElementInst&)instruction, state);
    else if (llvm::isa<llvm::GetElementPtrInst>(instruction))
        getelementptr((const llvm::GetElementPtrInst&)instruction, state);
    else if (llvm::isa<llvm::InsertElementInst>(instruction))
        insertelement((const llvm::InsertElementInst&)instruction, state);
    else if (llvm::isa<llvm::InsertValueInst>(instruction))
        insertvalue((const llvm::InsertValueInst&)instruction, state);
#if LLVM_VERSION_MAJOR >= 3
    // Instructions available since LLVM 3.0
    else if (llvm::isa<llvm::LandingPadInst>(instruction))
        landingpad((const llvm::LandingPadInst&)instruction, state);
    else if (llvm::isa<llvm::AtomicCmpXchgInst>(instruction))
        cmpxchg((const llvm::AtomicCmpXchgInst&)instruction, state);
    else if (llvm::isa<llvm::AtomicRMWInst>(instruction))
        atomicrmw((const llvm::AtomicRMWInst&)instruction, state);
    else if (llvm::isa<llvm::FenceInst>(instruction))
        fence((const llvm::FenceInst&)instruction, state);
#endif
    else if (llvm::isa<llvm::PHINode>(instruction))
        phi((const llvm::PHINode&)instruction, state);
    else if (llvm::isa<llvm::SelectInst>(instruction))
        select((const llvm::SelectInst&)instruction, state);
    else if (llvm::isa<llvm::ShuffleVectorInst>(instruction))
        shufflevector((const llvm::ShuffleVectorInst&)instruction, state);
    else if (llvm::isa<llvm::StoreInst>(instruction))
        store((const llvm::StoreInst&)instruction, state);
    else if (llvm::isa<llvm::UnaryInstruction>(instruction))
    {
        if (llvm::isa<llvm::AllocaInst>(instruction))
            alloca_((const llvm::AllocaInst&)instruction, state);
        else if (llvm::isa<llvm::CastInst>(instruction))
        {
            if (llvm::isa<llvm::BitCastInst>(instruction))
                bitcast((const llvm::BitCastInst&)instruction, state);
            else if (llvm::isa<llvm::FPExtInst>(instruction))
                fpext((const llvm::FPExtInst&)instruction, state);
            else if (llvm::isa<llvm::FPToSIInst>(instruction))
                fptosi((const llvm::FPToSIInst&)instruction, state);
            else if (llvm::isa<llvm::FPToUIInst>(instruction))
                fptoui((const llvm::FPToUIInst&)instruction, state);
            else if (llvm::isa<llvm::FPTruncInst>(instruction))
                fptrunc((const llvm::FPTruncInst&)instruction, state);
            else if (llvm::isa<llvm::IntToPtrInst>(instruction))
                inttoptr((const llvm::IntToPtrInst&)instruction, state);
            else if (llvm::isa<llvm::PtrToIntInst>(instruction))
                ptrtoint((const llvm::PtrToIntInst&)instruction, state);
            else if (llvm::isa<llvm::SExtInst>(instruction))
                sext((const llvm::SExtInst&)instruction, state);
            else if (llvm::isa<llvm::SIToFPInst>(instruction))
                sitofp((const llvm::SIToFPInst&)instruction, state);
            else if (llvm::isa<llvm::TruncInst>(instruction))
                trunc((const llvm::TruncInst&)instruction, state);
            else if (llvm::isa<llvm::UIToFPInst>(instruction))
                uitofp((const llvm::UIToFPInst&)instruction, state);
            else if (llvm::isa<llvm::ZExtInst>(instruction))
                zext((const llvm::ZExtInst&)instruction, state);
            else
                CANAL_FATAL_ERROR("Unknown cast instruction: " << instruction);
        }
        else if (llvm::isa<llvm::ExtractValueInst>(instruction))
            extractvalue((const llvm::ExtractValueInst&)instruction, state);
        else if (llvm::isa<llvm::LoadInst>(instruction))
            load((const llvm::LoadInst&)instruction, state);
        else if (llvm::isa<llvm::VAArgInst>(instruction))
            va_arg_((const llvm::VAArgInst&)instruction, state);
        else
            CANAL_FATAL_ERROR("Unknown unary instruction: " << instruction);
    }
    else if (llvm::isa<llvm::TerminatorInst>(instruction))
    {
        if (llvm::isa<llvm::BranchInst>(instruction))
            br((const llvm::BranchInst&)instruction, state);
        else if (llvm::isa<llvm::IndirectBrInst>(instruction))
            indirectbr((const llvm::IndirectBrInst&)instruction, state);
        else if (llvm::isa<llvm::InvokeInst>(instruction))
            invoke((const llvm::InvokeInst&)instruction, state);
#if LLVM_VERSION_MAJOR >= 3
        // Resume instruction is available since LLVM 3.0
        else if (llvm::isa<llvm::ResumeInst>(instruction))
            resume((const llvm::ResumeInst&)instruction, state);
#endif
        else if (llvm::isa<llvm::ReturnInst>(instruction))
            ret((const llvm::ReturnInst&)instruction, state);
        else if (llvm::isa<llvm::SwitchInst>(instruction))
            switch_((const llvm::SwitchInst&)instruction, state);
        else if (llvm::isa<llvm::UnreachableInst>(instruction))
            unreachable((const llvm::UnreachableInst&)instruction, state);
        else
            CANAL_FATAL_ERROR("Unknown terminator instruction: " << instruction);
    }
    else if (llvm::isa<llvm::BinaryOperator>(instruction))
    {
        const llvm::BinaryOperator &binaryOp = (const llvm::BinaryOperator&)instruction;
        assert(binaryOp.getNumOperands() == 2);
        switch (binaryOp.getOpcode())
	{
	case llvm::Instruction::Add:  add(binaryOp, state);  break;
	case llvm::Instruction::FAdd: fadd(binaryOp, state); break;
	case llvm::Instruction::Sub:  sub(binaryOp, state);  break;
	case llvm::Instruction::FSub: fsub(binaryOp, state); break;
	case llvm::Instruction::Mul:  mul(binaryOp, state);  break;
	case llvm::Instruction::FMul: fmul(binaryOp, state); break;
	case llvm::Instruction::UDiv: udiv(binaryOp, state); break;
	case llvm::Instruction::SDiv: sdiv(binaryOp, state); break;
	case llvm::Instruction::FDiv: fdiv(binaryOp, state); break;
	case llvm::Instruction::URem: urem(binaryOp, state); break;
	case llvm::Instruction::SRem: srem(binaryOp, state); break;
	case llvm::Instruction::FRem: frem(binaryOp, state); break;
	case llvm::Instruction::Shl:  shl(binaryOp, state);  break;
	case llvm::Instruction::LShr: lshr(binaryOp, state); break;
	case llvm::Instruction::AShr: ashr(binaryOp, state); break;
	case llvm::Instruction::And:  and_(binaryOp, state); break;
	case llvm::Instruction::Or:   or_(binaryOp, state);  break;
	case llvm::Instruction::Xor:  xor_(binaryOp, state); break;
	default:
            CANAL_FATAL_ERROR(binaryOp);
	}
    }
    else
        CANAL_FATAL_ERROR("Unknown instruction: "
                          << instruction.getOpcodeName());
}

const Domain *
Operations::variableOrConstant(const llvm::Value &place,
                               State &state,
                               const llvm::Instruction &instruction,
                               llvm::OwningPtr<Domain> &constant) const
{
    const Domain *variable = state.findVariable(place);
    if (variable)
        return variable;

    if (llvm::isa<llvm::Constant>(place))
    {
        Domain *constValue = mConstructors.create(llvmCast<llvm::Constant>(place),
                                                  instruction,
                                                  &state);

        llvm::OwningPtr<Domain> ptr(constValue);
        constant.swap(ptr);
        return constValue;
    }

    return NULL;
}

template<typename T> void
Operations::interpretCall(const T &instruction,
                          State &state)
{
    llvm::Function *function = instruction.getCalledFunction();
    CANAL_ASSERT(function);

    // Create the calling state.
    State callingState;
    callingState.mergeGlobal(state);

    // TODO: not all function blocks should be merged to the state.
    // Only the function blocks accessible from the arguments and
    // global variables should be merged.
    callingState.mergeFunctionBlocks(state);

    // Add function arguments to the calling state.
    llvm::Function::ArgumentListType::const_iterator it =
        function->getArgumentList().begin();

    unsigned arg = 0;
    for (; arg < function->getArgumentList().size(); ++arg, ++it)
    {
        llvm::Value *operand = instruction.getArgOperand(arg);

        llvm::OwningPtr<Domain> constant;
        const Domain *value = variableOrConstant(*operand,
                                                 state,
                                                 instruction,
                                                 constant);

        if (!value)
            return;

        callingState.addFunctionVariable(*it, value->clone());
    }

    for (; arg < instruction.getNumArgOperands(); ++arg)
    {
        llvm::Value *operand = instruction.getArgOperand(arg);

        llvm::OwningPtr<Domain> constant;
        const Domain *value = variableOrConstant(*operand,
                                                 state,
                                                 instruction,
                                                 constant);

        if (!value)
            return;

        callingState.addVariableArgument(instruction, value->clone());
    }

    mCallback.onFunctionCall(*function,
                             callingState,
                             state,
                             instruction);
}

void
Operations::binaryOperation(const llvm::BinaryOperator &instruction,
                            State &state,
                            Domain::BinaryOperation operation)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           instruction,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           instruction,
                           constants[1])
    };
    if (!values[0] || !values[1])
        return;

    // Create result value of required type and then run the desired
    // operation.
    Domain *result = mConstructors.create(*instruction.getType());
    ((result)->*(operation))(*values[0], *values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

template<typename T> bool
Operations::getElementPtrOffsets(std::vector<Domain*> &result,
                                 T iteratorStart,
                                 T iteratorEnd,
                                 const llvm::Value &place,
                                 const State &state)
{
    // Check that all variables exist before building the offset list.
    for (T it = iteratorStart; it != iteratorEnd; ++it)
    {
        if (llvm::isa<llvm::ConstantInt>(it))
            continue;

        const Domain *offset = state.findVariable(*it->get());
        // Not all offsets are necesarily known at each pass before
        // reaching a fixpoint.
        if (!offset)
            return false;
    }

    // Build the offset list.
    for (T it = iteratorStart; it != iteratorEnd; ++it)
    {
        if (llvm::isa<llvm::ConstantInt>(it))
        {
            const llvm::ConstantInt *constant =
                llvmCast<llvm::ConstantInt>(it);

            result.push_back(mConstructors.create(*constant,
                                                  place,
                                                  &state));
        }
        else
        {
            const Domain *offset = state.findVariable(*it->get());
            result.push_back(offset->clone());
        }
    }

    // Extend all values to 64 bits.
    std::vector<Domain*>::iterator resultIt = result.begin();
    for (; resultIt != result.end(); ++resultIt)
    {
        unsigned bitWidth = Integer::Utils::getBitWidth(**resultIt);

        CANAL_ASSERT_MSG(bitWidth <= 64,
                         "Cannot handle GetElementPtr offset"
                         " with more than 64 bits.");

        if (bitWidth != 64)
        {
            Domain *extended = mConstructors.createInteger(64);

            extended->sext(**resultIt);
            delete *resultIt;
            *resultIt = extended;
        }
    }

    return true;
}

void
Operations::castOperation(const llvm::CastInst &instruction,
                          State &state,
                          Domain::CastOperation operation)
{
    llvm::OwningPtr<Domain> constant;
    const Domain *source = variableOrConstant(
        *instruction.getOperand(0),
        state,
        instruction,
        constant);

    if (!source)
        return;

    // Create result value of required type and then run the desired
    // operation.
    Domain *result = mConstructors.create(*instruction.getDestTy());
    ((result)->*(operation))(*source);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void
Operations::cmpOperation(const llvm::CmpInst &instruction,
                         State &state,
                         Domain::CmpOperation operation)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           instruction,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           instruction,
                           constants[1])
    };

    if (!values[0] || !values[1])
        return;

    Domain *resultValue = mConstructors.create(*instruction.getType());
    ((resultValue)->*(operation))(*values[0],
                                  *values[1],
                                  instruction.getPredicate());

    state.addFunctionVariable(instruction, resultValue);
}

void
Operations::ret(const llvm::ReturnInst &instruction,
                State &state)
{
    llvm::Value *value = instruction.getReturnValue();
    // Return value is optional, some functions return nothing.
    if (!value)
        return;

    llvm::OwningPtr<Domain> constant;
    const Domain *variable = variableOrConstant(*value,
                                                state,
                                                instruction,
                                                constant);

    if (variable)
        state.mergeToReturnedValue(*variable);
}

void
Operations::br(const llvm::BranchInst &instruction,
               State &state)
{
    // Ignore.
}

void
Operations::switch_(const llvm::SwitchInst &instruction,
                    State &state)
{
    // Ignore.
}

void
Operations::indirectbr(const llvm::IndirectBrInst &instruction,
                       State &state)
{
    // Ignore.
}

void
Operations::invoke(const llvm::InvokeInst &instruction,
                   State &state)
{
    interpretCall(instruction, state);
}

void
Operations::unreachable(const llvm::UnreachableInst &instruction,
                        State &state)
{
    // Ignore.
}

void Operations::add(const llvm::BinaryOperator &instruction,
                     State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *a = variableOrConstant(*instruction.getOperand(0),
                                         state,
                                         instruction,
                                         constants[0]);

    const Domain *b = variableOrConstant(*instruction.getOperand(1),
                                         state,
                                         instruction,
                                         constants[1]);

    if (!a || !b)
        return;

    // Pointer arithmetic.
    const Pointer::Pointer *aPointer =
        dynCast<const Pointer::Pointer*>(a);

    const Pointer::Pointer *bPointer =
        dynCast<const Pointer::Pointer*>(b);

    CANAL_ASSERT_MSG(!aPointer || !bPointer,
                     "Unable to add two pointers.");

    Domain *result = NULL;
    // Pointer addition.
    if (aPointer || bPointer)
    {
        if (aPointer)
            result = mConstructors.createPointer(aPointer->getValueType());
        else
            result = mConstructors.createPointer(bPointer->getValueType());

        result->add(*a, *b);
    }
    else
    {
        // Create result value of required type and then run the desired
        // operation.
        result = mConstructors.create(*instruction.getType());
        result->add(*a, *b);
    }

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void
Operations::fadd(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::fadd);
}

void
Operations::sub(const llvm::BinaryOperator &instruction,
                State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *a = variableOrConstant(*instruction.getOperand(0),
                                         state,
                                         instruction,
                                         constants[0]);

    const Domain *b = variableOrConstant(*instruction.getOperand(1),
                                         state,
                                         instruction,
                                         constants[1]);

    if (!a || !b)
        return;

    // Pointer arithmetic.
    const Pointer::Pointer *aPointer =
        dynCast<const Pointer::Pointer*>(a);

    const Pointer::Pointer *bPointer =
        dynCast<const Pointer::Pointer*>(b);

    CANAL_ASSERT_MSG(aPointer || !bPointer,
                     "Subtracting pointer from constant!");

    Domain *result = NULL;

    // Subtracting integer from pointer.
    if (aPointer && !bPointer)
    {
        result = mConstructors.createPointer(aPointer->getValueType());
        result->sub(*a, *b);
    }
    else if (aPointer && bPointer) // Subtracting two pointers.
    {
        // We set to top for now.
        result = mConstructors.create(*instruction.getType());
        result->setTop();
    }
    else
    {
        // Create result value of required type and then run the desired
        // operation.
        result = mConstructors.create(*instruction.getType());
        result->sub(*a, *b);
    }

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void
Operations::fsub(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::fsub);
}

void
Operations::mul(const llvm::BinaryOperator &instruction,
                State &state)
{
    binaryOperation(instruction, state, &Domain::mul);
}

void
Operations::fmul(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::fmul);
}

void
Operations::udiv(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::udiv);
}

void
Operations::sdiv(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::sdiv);
}

void
Operations::fdiv(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::fdiv);
}

void
Operations::urem(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::urem);
}

void
Operations::srem(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::srem);
}

void
Operations::frem(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::frem);
}

void
Operations::shl(const llvm::BinaryOperator &instruction,
                State &state)
{
    binaryOperation(instruction, state, &Domain::shl);
}

void
Operations::lshr(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::lshr);
}

void
Operations::ashr(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::ashr);
}

void
Operations::and_(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::and_);
}

void
Operations::or_(const llvm::BinaryOperator &instruction,
                State &state)
{
    binaryOperation(instruction, state, &Domain::or_);
}

void
Operations::xor_(const llvm::BinaryOperator &instruction,
                 State &state)
{
    binaryOperation(instruction, state, &Domain::xor_);
}

void
Operations::extractelement(const llvm::ExtractElementInst &instruction,
                           State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.  Fixpoint
    // calculation is probably not far enough.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           instruction,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           instruction,
                           constants[1])
    };

    if (!values[0] || !values[1])
        return;

    Domain *result = mConstructors.create(*instruction.getType());
    result->extractelement(*values[0], *values[1]);
    state.addFunctionVariable(instruction, result);
}

void
Operations::insertelement(const llvm::InsertElementInst &instruction,
                          State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.  Fixpoint
    // calculation is probably not far enough.
    llvm::OwningPtr<Domain> constants[3];
    const Domain *values[3] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           instruction,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           instruction,
                           constants[1]),
        variableOrConstant(*instruction.getOperand(2),
                           state,
                           instruction,
                           constants[2])
    };

    if (!values[0] || !values[1] || !values[2])
        return;

    Domain *result = mConstructors.create(*instruction.getType());
    result->insertelement(*values[0], *values[1], *values[2]);
    state.addFunctionVariable(instruction, result);
}

void
Operations::shufflevector(const llvm::ShuffleVectorInst &instruction,
                          State &state)
{
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           instruction,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           instruction,
                           constants[1])
    };

    if (!values[0] || !values[1])
        return;

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 1
    llvm::SmallVector<int, 16> shuffleMaskLlvm =
        instruction.getShuffleMask();

    std::vector<uint32_t> shuffleMask(shuffleMaskLlvm.begin(),
                                      shuffleMaskLlvm.end());
#else
    std::vector<uint32_t> shuffleMask;
    {
        // Reimplementation of the missing getShuffleMask method.
        const llvm::Value *inputMask = instruction.getOperand(2);
        CANAL_ASSERT_MSG(inputMask, "Failed to get shufflevector mask.");
        const llvm::VectorType *inputMaskType =
            llvmCast<llvm::VectorType>(inputMask->getType());

        unsigned count = inputMaskType->getNumElements();
        const llvm::ConstantVector *inputMaskConstant =
            llvmCast<llvm::ConstantVector>(inputMask);

        for (unsigned i = 0; i != count; ++i)
        {
            llvm::Constant *constant = inputMaskConstant->getOperand(i);
            uint32_t constantInt = llvm::isa<llvm::UndefValue>(constant) ? -1 :
                llvmCast<llvm::ConstantInt>(constant)->getZExtValue();

            shuffleMask.push_back(constantInt);
        }
    }
#endif

    Domain *result = mConstructors.create(*instruction.getType());
    result->shufflevector(*values[0], *values[1], shuffleMask);
    state.addFunctionVariable(instruction, result);
}

void
Operations::extractvalue(const llvm::ExtractValueInst &instruction,
                         State &state)
{
    llvm::OwningPtr<Domain> constant;
    const Domain *aggregate = variableOrConstant(
        *instruction.getAggregateOperand(),
        state,
        instruction,
        constant);

    if (!aggregate)
        return;

    llvm::ArrayRef<unsigned> indicesLlvm = instruction.getIndices();
    std::vector<unsigned> indices(indicesLlvm.begin(),
                                  indicesLlvm.end());

    Domain *result = mConstructors.create(*instruction.getType());
    result->extractvalue(*aggregate, indices);
    state.addFunctionVariable(instruction, result);
}

void
Operations::insertvalue(const llvm::InsertValueInst &instruction,
                        State &state)
{
    llvm::OwningPtr<Domain> aggregateConstant;
    const Domain *aggregate = variableOrConstant(
        *instruction.getAggregateOperand(),
        state,
        instruction,
        aggregateConstant);

    if (!aggregate)
        return;

    llvm::OwningPtr<Domain> insertedConstant;
    const Domain *insertedValue = variableOrConstant(
        *instruction.getInsertedValueOperand(),
        state,
        instruction,
        insertedConstant);

    if (!insertedValue)
        return;

    llvm::ArrayRef<unsigned> indicesLlvm = instruction.getIndices();
    std::vector<unsigned> indices(indicesLlvm.begin(),
                                  indicesLlvm.end());

    Domain *result = mConstructors.create(*instruction.getType());
    result->insertvalue(*aggregate, *insertedValue, indices);
    state.addFunctionVariable(instruction, result);
}

void
Operations::alloca_(const llvm::AllocaInst &instruction,
                    State &state)
{
    Domain *value;
    std::vector<Domain*> offsets;

    if (instruction.isArrayAllocation())
    {
        const llvm::Value &arraySize = *instruction.getArraySize();
        llvm::OwningPtr<Domain> constant;
        const Domain *abstractSize = variableOrConstant(arraySize,
                                                        state,
                                                        instruction,
                                                        constant);

        if (!abstractSize)
            return;

        value = mConstructors.createArray(*instruction.getType(),
                                          abstractSize->clone());

        // Set pointer offset.
        Domain *zero = mConstructors.createInteger(llvm::APInt(64, 0));
        offsets.push_back(zero);
        offsets.push_back(zero->clone());
    }
    else
    {
        const llvm::Type &allocatedType = *instruction.getAllocatedType();
        value = mConstructors.create(allocatedType);
    }

    state.addFunctionBlock(instruction, new Memory(value));

    Domain *pointer;
    pointer = mConstructors.createPointer(*instruction.getType());
    Pointer::Utils::addTarget(*pointer,
                              Pointer::Target::Block,
                              &instruction,
                              &instruction,
                              offsets,
                              NULL);

    state.addFunctionVariable(instruction, pointer);
}

void
Operations::load(const llvm::LoadInst &instruction,
                 State &state)
{
    // Find the pointer in the state.  If the pointer is not
    // available, do nothing.
    const Domain *variable = state.findVariable(
        *instruction.getPointerOperand());

    if (!variable)
        return;

    const Pointer::Pointer &pointer =
        dynCast<const Pointer::Pointer&>(*variable);

    // Pointer found. Merge all possible values and store the result
    // into the state.
    Domain *mergedValue = pointer.load(state);
    if (!mergedValue)
        return;

    state.addFunctionVariable(instruction, mergedValue);
}

void
Operations::store(const llvm::StoreInst &instruction,
                  State &state)
{
    llvm::OwningPtr<Domain> constantPointer, constantValue;
    const Domain *pointer = variableOrConstant(
        *instruction.getPointerOperand(),
        state,
        instruction,
        constantPointer);

    const Domain *value = variableOrConstant(
        *instruction.getValueOperand(),
        state,
        instruction,
        constantValue);

    if (!pointer || !value)
        return;

    const Pointer::Pointer &inclusionBased =
        dynCast<const Pointer::Pointer&>(*pointer);

    inclusionBased.store(*value, state);
}

void
Operations::getelementptr(const llvm::GetElementPtrInst &instruction,
                          State &state)
{
    CANAL_ASSERT(instruction.getNumOperands() > 1);

    // Find the base pointer.
    const Domain *base = state.findVariable(*instruction.getPointerOperand());
    if (!base)
        return;

    const Pointer::Pointer &source =
        dynCast<const Pointer::Pointer&>(*base);

    // We get offsets. Either constants or Integer::Container.
    // Pointer points either to an array (or array offset), or to a
    // struct (or struct member).  Pointer might have multiple
    // targets.
    std::vector<Domain*> offsets;
    bool allOffsetsPresent = getElementPtrOffsets(offsets,
                                                  instruction.idx_begin(),
                                                  instruction.idx_end(),
                                                  instruction,
                                                  state);

    if (!allOffsetsPresent)
        return;

    const llvm::PointerType *pointerType = instruction.getType();
    CANAL_ASSERT(pointerType);
    Pointer::Pointer *result = source.getElementPtr(offsets, *pointerType);
    state.addFunctionVariable(instruction, result);
}

void
Operations::trunc(const llvm::TruncInst &instruction,
                  State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::trunc);
}

void
Operations::zext(const llvm::ZExtInst &instruction,
                 State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::zext);
}

void
Operations::sext(const llvm::SExtInst &instruction,
                 State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::sext);
}

void
Operations::fptrunc(const llvm::FPTruncInst &instruction,
                    State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fptrunc);
}

void
Operations::fpext(const llvm::FPExtInst &instruction,
                  State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fpext);
}

void
Operations::fptoui(const llvm::FPToUIInst &instruction,
                   State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fptoui);
}

void
Operations::fptosi(const llvm::FPToSIInst &instruction,
                   State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fptosi);
}

void
Operations::uitofp(const llvm::UIToFPInst &instruction,
                   State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::uitofp);
}

void
Operations::sitofp(const llvm::SIToFPInst &instruction,
                    State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::sitofp);
}

void
Operations::ptrtoint(const llvm::PtrToIntInst &instruction,
                     State &state)
{
    const Domain *operand =
        state.findVariable(*instruction.getOperand(0));

    if (!operand)
        return;

    return state.addFunctionVariable(instruction, operand->clone());
}

void
Operations::inttoptr(const llvm::IntToPtrInst &instruction,
                     State &state)
{
    const Domain *operand = state.findVariable(*instruction.getOperand(0));
    if (!operand)
        return;

    const Pointer::Pointer &source =
        dynCast<const Pointer::Pointer&>(*operand);

    const llvm::PointerType &pointerType =
        llvmCast<const llvm::PointerType>(*instruction.getDestTy());

    Domain *result = new Pointer::Pointer(source, pointerType);
    state.addFunctionVariable(instruction, result);
}

void
Operations::bitcast(const llvm::BitCastInst &instruction,
                    State &state)
{
    const llvm::Type *sourceType = instruction.getSrcTy();
    const llvm::Type *destinationType = instruction.getDestTy();

    CANAL_ASSERT_MSG(sourceType->isPointerTy() &&
                     destinationType->isPointerTy(),
                     "Bitcast for non-pointers is not implemented yet.");

    const Domain *source = state.findVariable(*instruction.getOperand(0));
    if (!source)
        return;

    const Pointer::Pointer &sourcePointer =
        dynCast<const Pointer::Pointer&>(*source);

    const llvm::PointerType &destPointerType =
        llvmCast<const llvm::PointerType>(*destinationType);

    Domain *result = new Pointer::Pointer(sourcePointer,
                                          destPointerType);

    state.addFunctionVariable(instruction, result);
}

void
Operations::icmp(const llvm::ICmpInst &instruction,
                 State &state)
{
    cmpOperation(instruction, state, &Domain::icmp);
}

void
Operations::fcmp(const llvm::FCmpInst &instruction,
                 State &state)
{
    cmpOperation(instruction, state, &Domain::fcmp);
}

void
Operations::phi(const llvm::PHINode &instruction,
                State &state)
{
    Domain *mergedValue = NULL;
    for (unsigned i = 0; i < instruction.getNumIncomingValues(); ++i)
    {
        llvm::OwningPtr<Domain> constant;
        const Domain *value = variableOrConstant(
            *instruction.getIncomingValue(i),
            state,
            instruction,
            constant);

        if (!value)
            continue;

        if (mergedValue)
            mergedValue->join(*value);
        else
            mergedValue = value->clone();
    }

    if (!mergedValue)
        return;

    state.addFunctionVariable(instruction, mergedValue);
}

void
Operations::select(const llvm::SelectInst &instruction,
                   State &state)
{
    const Domain *condition = state.findVariable(*instruction.getCondition());
    if (!condition)
        return;

    llvm::OwningPtr<Domain> trueConstant, falseConstant;

    const Domain *trueValue = variableOrConstant(
        *instruction.getTrueValue(),
        state,
        instruction,
        trueConstant);

    const Domain *falseValue = variableOrConstant(
        *instruction.getFalseValue(),
        state,
        instruction,
        falseConstant);

    Domain *resultValue;
    const Integer::Container &conditionInt =
        dynCast<const Integer::Container&>(*condition);

    CANAL_ASSERT(Integer::Utils::getBitfield(conditionInt).getBitWidth() == 1);
    switch (Integer::Utils::getBitfield(conditionInt).getBitValue(0))
    {
    case -1:
        // The condition result is undefined.  Let's wait for
        // another iteration.
        return;
    case 0:
        resultValue = falseValue->clone();
        break;
    case 1:
        resultValue = trueValue->clone();
        break;
    case 2:
        // Both true and false results are possible.
        resultValue = trueValue->clone();
        resultValue->join(*falseValue);
        break;
    default:
        CANAL_DIE();
    }

    state.addFunctionVariable(instruction, resultValue);
}

void
Operations::call(const llvm::CallInst &instruction,
                 State &state)
{
    interpretCall(instruction, state);
}

void
Operations::va_arg_(const llvm::VAArgInst &instruction,
                   State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

#if LLVM_VERSION_MAJOR >= 3
// Instructions available since LLVM 3.0

void
Operations::resume(const llvm::ResumeInst &instruction,
                   State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::fence(const llvm::FenceInst &instruction,
                  State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::cmpxchg(const llvm::AtomicCmpXchgInst &instruction,
                    State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::atomicrmw(const llvm::AtomicRMWInst &instruction,
                      State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::landingpad(const llvm::LandingPadInst &instruction,
                       State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

#endif

} // namespace Canal
