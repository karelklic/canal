#include "Operations.h"
#include "ArrayExactSize.h"
#include "ArraySingleItem.h"
#include "Constructors.h"
#include "Environment.h"
#include "FloatInterval.h"
#include "IntegerBitfield.h"
#include "ProductVector.h"
#include "IntegerUtils.h"
#include "OperationsCallback.h"
#include "MemoryPointer.h"
#include "MemoryUtils.h"
#include "Structure.h"
#include "Utils.h"
#include "Domain.h"
#include "MemoryState.h"
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
                                 Memory::State &state)
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
                               Memory::State &state,
                               llvm::OwningPtr<Domain> &constant) const
{
    const Domain *variable = state.findVariable(place);
    if (variable)
        return variable;

    if (llvm::isa<llvm::Constant>(place))
    {
        Domain *constValue = mConstructors.create(checkedCast<llvm::Constant>(place),
                                                  &state);

        llvm::OwningPtr<Domain> ptr(constValue);
        constant.swap(ptr);
        return constValue;
    }

    return NULL;
}

template<typename T> void
Operations::interpretCall(const T &instruction,
                          Memory::State &state)
{
    llvm::Function *function = instruction.getCalledFunction();
    CANAL_ASSERT(function);

    // Create the calling state.
    Memory::State callingState;
    callingState.joinGlobal(state);

    // TODO: not all function blocks should be merged to the state.
    // Only the function blocks accessible from the arguments and
    // global variables should be merged.
    callingState.joinStackBlocks(state);

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
                            Memory::State &state,
                            Domain::BinaryOperation operation)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
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

bool
Operations::getElementPtrOffsets(std::vector<const Domain*> &result,
                                 llvm::GetElementPtrInst::const_op_iterator iteratorStart,
                                 llvm::GetElementPtrInst::const_op_iterator iteratorEnd,
                                 const llvm::Value &place,
                                 const Memory::State &state)
{
    // Check that all variables exist before building the offset list.
    for (llvm::GetElementPtrInst::const_op_iterator it = iteratorStart;
         it != iteratorEnd;
         ++it)
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
    for (llvm::GetElementPtrInst::const_op_iterator it = iteratorStart;
         it != iteratorEnd;
         ++it)
    {
        if (llvm::isa<llvm::ConstantInt>(it))
        {
            const llvm::ConstantInt *constant =
                checkedCast<llvm::ConstantInt>(it);

            result.push_back(mConstructors.create(*constant,
                                                  &state));
        }
        else
        {
            const Domain *offset = state.findVariable(*it->get());
            result.push_back(offset->clone());
        }
    }

    // Extend all values to 64 bits.
    std::vector<const Domain*>::iterator resultIt = result.begin();
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
                          Memory::State &state,
                          Domain::CastOperation operation)
{
    llvm::OwningPtr<Domain> constant;
    const Domain *source = variableOrConstant(
        *instruction.getOperand(0),
        state,
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
                         Memory::State &state,
                         Domain::CmpOperation operation)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
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
                Memory::State &state)
{
    llvm::Value *value = instruction.getReturnValue();
    // Return value is optional, some functions return nothing.
    if (!value)
        return;

    llvm::OwningPtr<Domain> constant;
    const Domain *variable = variableOrConstant(*value,
                                                state,
                                                constant);

    if (variable)
        state.mergeToReturnedValue(*variable);
}

void
Operations::br(const llvm::BranchInst &instruction,
               Memory::State &state)
{
    // Ignore.
}

void
Operations::switch_(const llvm::SwitchInst &instruction,
                    Memory::State &state)
{
    // Ignore.
}

void
Operations::indirectbr(const llvm::IndirectBrInst &instruction,
                       Memory::State &state)
{
    // Ignore.
}

void
Operations::invoke(const llvm::InvokeInst &instruction,
                   Memory::State &state)
{
    interpretCall(instruction, state);
}

void
Operations::unreachable(const llvm::UnreachableInst &instruction,
                        Memory::State &state)
{
    // Ignore.
}

void Operations::add(const llvm::BinaryOperator &instruction,
                     Memory::State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *a = variableOrConstant(*instruction.getOperand(0),
                                         state,
                                         constants[0]);

    const Domain *b = variableOrConstant(*instruction.getOperand(1),
                                         state,
                                         constants[1]);

    if (!a || !b)
        return;

    // Pointer arithmetic.
    const Memory::Pointer *aPointer = dynCast<Memory::Pointer>(a);
    const Memory::Pointer *bPointer = dynCast<Memory::Pointer>(b);
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
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::fadd);
}

void
Operations::sub(const llvm::BinaryOperator &instruction,
                Memory::State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *a = variableOrConstant(*instruction.getOperand(0),
                                         state,
                                         constants[0]);

    const Domain *b = variableOrConstant(*instruction.getOperand(1),
                                         state,
                                         constants[1]);

    if (!a || !b)
        return;

    // Pointer arithmetic.
    const Memory::Pointer *aPointer = dynCast<Memory::Pointer>(a);
    const Memory::Pointer *bPointer = dynCast<Memory::Pointer>(b);
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
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::fsub);
}

void
Operations::mul(const llvm::BinaryOperator &instruction,
                Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::mul);
}

void
Operations::fmul(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::fmul);
}

void
Operations::udiv(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::udiv);
}

void
Operations::sdiv(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::sdiv);
}

void
Operations::fdiv(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::fdiv);
}

void
Operations::urem(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::urem);
}

void
Operations::srem(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::srem);
}

void
Operations::frem(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::frem);
}

void
Operations::shl(const llvm::BinaryOperator &instruction,
                Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::shl);
}

void
Operations::lshr(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::lshr);
}

void
Operations::ashr(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::ashr);
}

void
Operations::and_(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::and_);
}

void
Operations::or_(const llvm::BinaryOperator &instruction,
                Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::or_);
}

void
Operations::xor_(const llvm::BinaryOperator &instruction,
                 Memory::State &state)
{
    binaryOperation(instruction, state, &Domain::xor_);
}

void
Operations::extractelement(const llvm::ExtractElementInst &instruction,
                           Memory::State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.  Fixpoint
    // calculation is probably not far enough.
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           constants[1])
    };

    if (!values[0] || !values[1])
        return;

    Domain *result = values[0]->extractelement(*values[1]);
    state.addFunctionVariable(instruction, result);
}

void
Operations::insertelement(const llvm::InsertElementInst &instruction,
                          Memory::State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.  Fixpoint
    // calculation is probably not far enough.
    llvm::OwningPtr<Domain> constants[3];
    const Domain *values[3] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           constants[1]),
        variableOrConstant(*instruction.getOperand(2),
                           state,
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
                          Memory::State &state)
{
    llvm::OwningPtr<Domain> constants[2];
    const Domain *values[2] = {
        variableOrConstant(*instruction.getOperand(0),
                           state,
                           constants[0]),
        variableOrConstant(*instruction.getOperand(1),
                           state,
                           constants[1])
    };

    if (!values[0] || !values[1])
        return;

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR > 0) || LLVM_VERSION_MAJOR > 3
    llvm::SmallVector<int, 16> shuffleMaskSmallVector =
        instruction.getShuffleMask();

    std::vector<uint32_t> shuffleMask(shuffleMaskSmallVector.begin(),
                                      shuffleMaskSmallVector.end());
#else
    std::vector<uint32_t> shuffleMask;
    {
        // Reimplementation of the missing getShuffleMask method.
        const llvm::Value *inputMask = instruction.getOperand(2);
        CANAL_ASSERT_MSG(inputMask, "Failed to get shufflevector mask.");
        const llvm::VectorType *inputMaskType =
            checkedCast<llvm::VectorType>(inputMask->getType());

        unsigned count = inputMaskType->getNumElements();
        const llvm::ConstantVector *inputMaskConstant =
            checkedCast<llvm::ConstantVector>(inputMask);

        for (unsigned i = 0; i != count; ++i)
        {
            llvm::Constant *constant = inputMaskConstant->getOperand(i);
            uint32_t constantInt = llvm::isa<llvm::UndefValue>(constant) ? -1 :
                checkedCast<llvm::ConstantInt>(constant)->getZExtValue();

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
                         Memory::State &state)
{
    llvm::OwningPtr<Domain> constant;
    const Domain *aggregate = variableOrConstant(
        *instruction.getAggregateOperand(),
        state,
        constant);

    if (!aggregate)
        return;

    std::vector<unsigned> indices(instruction.idx_begin(),
                                  instruction.idx_end());

    Domain *result = aggregate->extractvalue(indices);
    state.addFunctionVariable(instruction, result);
}

void
Operations::insertvalue(const llvm::InsertValueInst &instruction,
                        Memory::State &state)
{
    llvm::OwningPtr<Domain> aggregateConstant;
    const Domain *aggregate = variableOrConstant(
        *instruction.getAggregateOperand(),
        state,
        aggregateConstant);

    if (!aggregate)
        return;

    llvm::OwningPtr<Domain> insertedConstant;
    const Domain *insertedValue = variableOrConstant(
        *instruction.getInsertedValueOperand(),
        state,
        insertedConstant);

    if (!insertedValue)
        return;

    std::vector<unsigned> indices(instruction.idx_begin(),
                                  instruction.idx_end());

    Domain *result = mConstructors.create(*instruction.getType());
    result->insertvalue(*aggregate, *insertedValue, indices);
    state.addFunctionVariable(instruction, result);
}

void
Operations::alloca_(const llvm::AllocaInst &instruction,
                    Memory::State &state)
{
    Domain *value;

    if (instruction.isArrayAllocation())
    {
        const llvm::Value &arraySize = *instruction.getArraySize();
        llvm::OwningPtr<Domain> constant;
        const Domain *abstractSize = variableOrConstant(arraySize,
                                                        state,
                                                        constant);

        if (!abstractSize)
            return;

        value = mConstructors.createArray(*instruction.getType(),
                                          abstractSize->clone());
    }
    else
    {
        const llvm::Type &allocatedType = *instruction.getAllocatedType();
        value = mConstructors.create(allocatedType);
    }

    Memory::Block *block = new Memory::Block(Memory::Block::StackMemoryType,
                                             value);

    state.addBlock(instruction, block);

    Domain *pointer;
    pointer = mConstructors.createPointer(*instruction.getType());
    Memory::Utils::addTarget(*pointer,
                             instruction,
                             NULL);

    state.addFunctionVariable(instruction, pointer);
}

void
Operations::load(const llvm::LoadInst &instruction,
                 Memory::State &state)
{
    // Find the pointer in the state.  If the pointer is not
    // available, do nothing.
    const Domain *variable = state.findVariable(
        *instruction.getPointerOperand());

    if (!variable)
        return;

    const Memory::Pointer &pointer =
        checkedCast<Memory::Pointer>(*variable);

    // Pointer found. Merge all possible values and store the result
    // into the state.
    Domain *mergedValue = pointer.dereferenceAndMerge(state);
    if (!mergedValue)
        return;

    state.addFunctionVariable(instruction, mergedValue);
}

void
Operations::store(const llvm::StoreInst &instruction,
                  Memory::State &state)
{
    llvm::OwningPtr<Domain> constantPointer, constantValue;
    const Domain *pointer = variableOrConstant(
        *instruction.getPointerOperand(),
        state,
        constantPointer);

    const Domain *value = variableOrConstant(
        *instruction.getValueOperand(),
        state,
        constantValue);

    if (!pointer || !value)
        return;

    const Memory::Pointer &inclusionBased =
        checkedCast<Memory::Pointer>(*pointer);

    inclusionBased.store(*value, state);
}

void
Operations::getelementptr(const llvm::GetElementPtrInst &instruction,
                          Memory::State &state)
{
    CANAL_ASSERT(instruction.getNumOperands() > 1);

    // Find the source (base) pointer.
    const Domain *sourcePointer = state.findVariable(*instruction.getPointerOperand());
    if (!sourcePointer)
        return;

    const Memory::Pointer &source =
        checkedCast<Memory::Pointer>(*sourcePointer);

    // We get offsets. Either constants or Product::Vector.
    // Pointer points either to an array (or array offset), or to a
    // struct (or struct member).  Pointer might have multiple
    // targets.
    std::vector<const Domain*> offsets;
    bool allOffsetsPresent = getElementPtrOffsets(offsets,
                                                  instruction.idx_begin(),
                                                  instruction.idx_end(),
                                                  instruction,
                                                  state);

    if (!allOffsetsPresent)
        return;

    Domain *byteOffset = Memory::Utils::getByteOffset(offsets.begin(),
                                                      offsets.end(),
                                                      source.getValueType(),
                                                      mEnvironment);

    llvm::DeleteContainerPointers(offsets);
    const llvm::PointerType &pointerType = *instruction.getType();
    Memory::Pointer *result = source.withOffset(*byteOffset, pointerType);
    delete byteOffset;
    state.addFunctionVariable(instruction, result);
}

void
Operations::trunc(const llvm::TruncInst &instruction,
                  Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::trunc);
}

void
Operations::zext(const llvm::ZExtInst &instruction,
                 Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::zext);
}

void
Operations::sext(const llvm::SExtInst &instruction,
                 Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::sext);
}

void
Operations::fptrunc(const llvm::FPTruncInst &instruction,
                    Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fptrunc);
}

void
Operations::fpext(const llvm::FPExtInst &instruction,
                  Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fpext);
}

void
Operations::fptoui(const llvm::FPToUIInst &instruction,
                   Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fptoui);
}

void
Operations::fptosi(const llvm::FPToSIInst &instruction,
                   Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::fptosi);
}

void
Operations::uitofp(const llvm::UIToFPInst &instruction,
                   Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::uitofp);
}

void
Operations::sitofp(const llvm::SIToFPInst &instruction,
                   Memory::State &state)
{
    castOperation(instruction,
                  state,
                  &Domain::sitofp);
}

void
Operations::ptrtoint(const llvm::PtrToIntInst &instruction,
                     Memory::State &state)
{
    const Domain *operand =
        state.findVariable(*instruction.getOperand(0));

    if (!operand)
        return;

    return state.addFunctionVariable(instruction, operand->clone());
}

void
Operations::inttoptr(const llvm::IntToPtrInst &instruction,
                     Memory::State &state)
{
    const Domain *operand = state.findVariable(*instruction.getOperand(0));
    if (!operand)
        return;

    const Memory::Pointer &source =
        checkedCast<Memory::Pointer>(*operand);

    const llvm::PointerType &pointerType =
        checkedCast<llvm::PointerType>(*instruction.getDestTy());

    Memory::Pointer *result = new Memory::Pointer(source, pointerType);
    state.addFunctionVariable(instruction, result);
}

void
Operations::bitcast(const llvm::BitCastInst &instruction,
                    Memory::State &state)
{
    const llvm::Type *sourceType = instruction.getSrcTy();
    const llvm::Type *destinationType = instruction.getDestTy();

    CANAL_ASSERT_MSG(sourceType->isPointerTy() &&
                     destinationType->isPointerTy(),
                     "Bitcast for non-pointers is not implemented yet.");

    const Domain *source = state.findVariable(*instruction.getOperand(0));
    if (!source)
        return;

    const Memory::Pointer &sourcePointer =
        checkedCast<Memory::Pointer>(*source);

    const llvm::PointerType &destPointerType =
        checkedCast<llvm::PointerType>(*destinationType);

    Domain *resultPointer = new Memory::Pointer(sourcePointer, destPointerType);
    state.addFunctionVariable(instruction, resultPointer);
}

void
Operations::icmp(const llvm::ICmpInst &instruction,
                 Memory::State &state)
{
    cmpOperation(instruction, state, &Domain::icmp);
}

void
Operations::fcmp(const llvm::FCmpInst &instruction,
                 Memory::State &state)
{
    cmpOperation(instruction, state, &Domain::fcmp);
}

void
Operations::phi(const llvm::PHINode &instruction,
                Memory::State &state)
{
    Domain *mergedValue = NULL;
    for (unsigned i = 0; i < instruction.getNumIncomingValues(); ++i)
    {
        llvm::OwningPtr<Domain> constant;
        const Domain *value = variableOrConstant(
            *instruction.getIncomingValue(i),
            state,
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
                   Memory::State &state)
{
    const Domain *condition = state.findVariable(*instruction.getCondition());
    if (!condition)
        return;

    llvm::OwningPtr<Domain> trueConstant, falseConstant;

    const Domain *trueValue = variableOrConstant(
        *instruction.getTrueValue(),
        state,
        trueConstant);

    const Domain *falseValue = variableOrConstant(
        *instruction.getFalseValue(),
        state,
        falseConstant);

    Domain *resultValue;
    const Product::Vector &conditionInt =
        checkedCast<Product::Vector>(*condition);

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
                 Memory::State &state)
{
    interpretCall(instruction, state);
}

void
Operations::va_arg_(const llvm::VAArgInst &instruction,
                   Memory::State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

#if LLVM_VERSION_MAJOR >= 3
// Instructions available since LLVM 3.0

void
Operations::resume(const llvm::ResumeInst &instruction,
                   Memory::State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::fence(const llvm::FenceInst &instruction,
                  Memory::State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::cmpxchg(const llvm::AtomicCmpXchgInst &instruction,
                    Memory::State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::atomicrmw(const llvm::AtomicRMWInst &instruction,
                      Memory::State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Operations::landingpad(const llvm::LandingPadInst &instruction,
                       Memory::State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

#endif

} // namespace Canal
