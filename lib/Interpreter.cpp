#include "Interpreter.h"
#include "State.h"
#include "Utils.h"
#include "Value.h"
#include "Integer.h"
#include "Pointer.h"
#include "Array.h"
#include "Float.h"
#include "Constant.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <llvm/Support/CFG.h>
#include <map>
#include <cassert>

namespace Canal {

Interpreter::Interpreter(llvm::Module &module) : mModule(module)
{
}

void
Interpreter::interpretFunction(const llvm::Function &function,
                               State &state)
{
    std::map<const llvm::BasicBlock*, State> blockInputState, blockOutputState;
    llvm::Function::const_iterator itBlock = function.begin(), itBlockEnd = function.end();
    for (; itBlock != itBlockEnd; ++itBlock)
    {
        blockInputState[itBlock] = state;
        blockOutputState[itBlock] = state;
    }

    interpretFunctionBlocks(function.begin(), itBlockEnd, blockInputState, blockOutputState);
}

void
Interpreter::interpretFunctionBlocks(llvm::Function::const_iterator blockBegin,
                                     llvm::Function::const_iterator blockEnd,
                                     std::map<const llvm::BasicBlock*, State> &blockInputState,
                                     std::map<const llvm::BasicBlock*, State> &blockOutputState)
{
    bool changed;
    do {
        changed = false;
        llvm::Function::const_iterator itBlock;
        for (itBlock = blockBegin; itBlock != blockEnd; ++itBlock)
        {
            // Merge out states of predecessors to input state of
            // current block.
            llvm::const_pred_iterator itPred = llvm::pred_begin(itBlock),
                itPredEnd = llvm::pred_end(itBlock);
            for (; itPred != itPredEnd; ++itPred)
            {
                assert(&*itBlock != &itBlock->getParent()->getEntryBlock() && "Entry block cannot have predecessors!");
                blockInputState[itBlock].merge(blockOutputState[*itPred]);
            }

            // Interpret all instructions of current block.
            State currentState(blockInputState[itBlock]);
            llvm::BasicBlock::const_iterator itInst = itBlock->begin(),
                itInstEnd = itBlock->end();
            for (; itInst != itInstEnd; ++itInst)
                interpretInstruction(*itInst, currentState);

            // Check if the state changed since the last pass of this
            // block.
	    llvm::outs() << "Comparing " << currentState << blockOutputState[itBlock] << "\n";
            if (currentState != blockOutputState[itBlock])
            {
                changed = true;
                blockOutputState[itBlock] = currentState;
		llvm::outs() << "Assigned " << blockOutputState[itBlock];
            }
        }
    } while (changed);
}

void
Interpreter::interpretInstruction(const llvm::Instruction &instruction, State &state)
{
    if (llvm::isa<llvm::AllocaInst>(instruction))
        alloca_((const llvm::AllocaInst&)instruction, state);
    else if (llvm::isa<llvm::StoreInst>(instruction))
        store((const llvm::StoreInst&)instruction, state);
    else if (llvm::isa<llvm::CallInst>(instruction))
        call((const llvm::CallInst&)instruction, state);
    else if (llvm::isa<llvm::LoadInst>(instruction))
        load((const llvm::LoadInst&)instruction, state);
    else if (llvm::isa<llvm::ICmpInst>(instruction))
        icmp((const llvm::ICmpInst&)instruction, state);
    else if (llvm::isa<llvm::FCmpInst>(instruction))
        fcmp((const llvm::FCmpInst&)instruction, state);
    else if (llvm::isa<llvm::GetElementPtrInst>(instruction))
        getelementptr((const llvm::GetElementPtrInst&)instruction, state);
    else if (llvm::isa<llvm::TerminatorInst>(instruction))
    {
        if (llvm::isa<llvm::ReturnInst>(instruction))
            ret((const llvm::ReturnInst&)instruction, state);
        else if (llvm::isa<llvm::BranchInst>(instruction))
            br((const llvm::BranchInst&)instruction, state);
        else if (llvm::isa<llvm::SwitchInst>(instruction))
            switch_((const llvm::SwitchInst&)instruction, state);
        else if (llvm::isa<llvm::IndirectBrInst>(instruction))
            indirectbr((const llvm::IndirectBrInst&)instruction, state);
        else if (llvm::isa<llvm::InvokeInst>(instruction))
            invoke((const llvm::InvokeInst&)instruction, state);
        else if (llvm::isa<llvm::ResumeInst>(instruction))
            resume((const llvm::ResumeInst&)instruction, state);
        else if (llvm::isa<llvm::UnreachableInst>(instruction))
            unreachable((const llvm::UnreachableInst&)instruction, state);
        else
            llvm::errs() << "Operational::Machine: Unknown terminator instruction: " << instruction << "\n";
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
        CANAL_DIE();
}

void
Interpreter::ret(const llvm::ReturnInst &instruction, State &state)
{
    llvm::Value *value = instruction.getReturnValue();
    Value *variable = state.findVariable(*value);
    // It might happen that the variable is not found in state,
    // because the function has not yet reached fixpoint.
    if (variable)
    {
        if (state.mReturnedValue)
            state.mReturnedValue->merge(*variable);
        else
            state.mReturnedValue = variable->clone();
    }
    else if (llvm::isa<llvm::Constant>(value))
    {
        if (state.mReturnedValue)
        {
            // If the returned value is constant, convert it to
            // something that can merge other values.
            Constant *constant = dynamic_cast<Constant*>(state.mReturnedValue);
            if (constant)
            {
                state.mReturnedValue = constant->toModifiableValue();
                delete constant;
            }

            Constant c(llvm::cast<llvm::Constant>(value));
            state.mReturnedValue->merge(c);
        }
        else
            state.mReturnedValue = new Constant(llvm::cast<llvm::Constant>(value));
    }
}

void
Interpreter::br(const llvm::BranchInst &instruction, State &state)
{
    // Ignore.
}

void
Interpreter::switch_(const llvm::SwitchInst &instruction, State &state)
{
    // Ignore.
}

void
Interpreter::indirectbr(const llvm::IndirectBrInst &instruction, State &state)
{
    // Ignore.
}

static Value *
variableOrConstant(const llvm::Value &place, State &state, Constant &constant)
{
    if (llvm::isa<llvm::Constant>(place))
    {
        constant.mConstant = llvm::cast<llvm::Constant>(&place);
        return &constant;
    }

    // Either NULL or existing variable.
    return state.findVariable(place);
}

template<typename T> static void
interpretCall(Interpreter &interpreter, const T &instruction, State &state)
{
    llvm::Function *function = instruction.getCalledFunction();
    State functionState(state);
    functionState.clearFunctionLevel();
    llvm::Function::ArgumentListType::const_iterator it = function->getArgumentList().begin();
    for (int i = 0; i < instruction.getNumArgOperands(); ++i, ++it)
    {
        llvm::Value *operand = instruction.getArgOperand(i);
        Constant c;
        Value *value = variableOrConstant(*operand, state, c);
        if (!value)
            return;
        functionState.addFunctionVariable(*it, value->clone());
    }

    interpreter.interpretFunction(*function, functionState);
    state.mergeGlobalLevel(functionState);
    if (functionState.mReturnedValue)
    {
        // Optimization: instead of cloning the returned value from
        // function state, we steal it (so it is not deleted in
        // functionState destructor).
        state.addFunctionVariable(instruction, functionState.mReturnedValue);
        functionState.mReturnedValue = NULL;
    }
}

void
Interpreter::invoke(const llvm::InvokeInst &instruction, State &state)
{
    interpretCall(*this, instruction, state);
}

void
Interpreter::resume(const llvm::ResumeInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::unreachable(const llvm::UnreachableInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

static void
binaryOperation(const llvm::BinaryOperator &instruction, State &state, void(Value::*operation)(const Value&, const Value&))
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    Constant constants[2];
    Value *values[2] = {
        variableOrConstant(*instruction.getOperand(0), state, constants[0]),
        variableOrConstant(*instruction.getOperand(1), state, constants[1])
    };
    if (!values[0] || !values[1])
        return;

    // Create result value of required type.
    llvm::Type *type = instruction.getType();
    Value *result = NULL;
    if (type->isIntegerTy())
    {
        llvm::IntegerType *integerType = llvm::cast<llvm::IntegerType>(type);
        result = new Integer::Container(integerType->getBitWidth());
    }
    else if (type->isFloatingPointTy())
    {
        const llvm::fltSemantics *semantics;
#if LLVM_MAJOR >= 3 && LLVM_MINOR >= 1
        if (type->isHalfTy())
            semantics = &llvm::APFloat::IEEEhalf;
        else
#endif
        if (type->isFloatTy())
            semantics = &llvm::APFloat::IEEEsingle;
        else if (type->isDoubleTy())
            semantics = &llvm::APFloat::IEEEdouble;
        else if (type->isFP128Ty())
            semantics = &llvm::APFloat::IEEEquad;
        else if (type->isPPC_FP128Ty())
            semantics = &llvm::APFloat::PPCDoubleDouble;
        else if (type->isX86_FP80Ty())
            semantics = &llvm::APFloat::x87DoubleExtended;
        else
            CANAL_DIE();

        result = new Float::Range(*semantics);
    }
    else
        CANAL_NOT_IMPLEMENTED();

    ((result)->*(operation))(*values[0], *values[1]);
    state.addFunctionVariable(instruction, result);
}

void Interpreter::add(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::add);
}

void
Interpreter::fadd(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::fadd);
}

void
Interpreter::sub(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::sub);
}

void
Interpreter::fsub(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::fsub);
}

void
Interpreter::mul(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::mul);
}

void
Interpreter::fmul(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::fmul);
}

void
Interpreter::udiv(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::udiv);
}

void
Interpreter::sdiv(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::sdiv);
}

void
Interpreter::fdiv(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::fdiv);
}

void
Interpreter::urem(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::urem);
}

void
Interpreter::srem(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::srem);
}

void
Interpreter::frem(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::frem);
}

void
Interpreter::shl(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::shl);
}

void
Interpreter::lshr(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::lshr);
}

void
Interpreter::ashr(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::ashr);
}

void
Interpreter::and_(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::and_);
}

void
Interpreter::or_(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::or_);
}

void
Interpreter::xor_(const llvm::BinaryOperator &instruction, State &state)
{
    binaryOperation(instruction, state, &Value::xor_);
}

void
Interpreter::extractelement(const llvm::ExtractElementInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::insertelement(const llvm::InsertElementInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::shufflevector(const llvm::ShuffleVectorInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::extractvalue(const llvm::ExtractValueInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::insertvalue(const llvm::InsertValueInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::alloca_(const llvm::AllocaInst &instruction, State &state)
{
    llvm::Type *type = instruction.getAllocatedType();
    Value *value = NULL;
    if (type->isIntegerTy())
    {
        llvm::IntegerType &integerType = llvm::cast<llvm::IntegerType>(*type);
        value = new Integer::Container(integerType.getBitWidth());
    }
    else if (type->isPointerTy())
        value = new Pointer::InclusionBased();
    else
        CANAL_DIE();

    if (instruction.isArrayAllocation())
    {
        Array::SingleItem *array = new Array::SingleItem();
        array->mItemValue = value;
        value = array;
        const llvm::Value *arraySize = instruction.getArraySize();
        PlaceValueMap::const_iterator it = state.getGlobalVariables().find(arraySize);
        if (it != state.getGlobalVariables().end())
            array->mSize = it->second->clone();
        else
        {
            it = state.getFunctionVariables().find(arraySize);
            if (it != state.getFunctionVariables().end())
                array->mSize = it->second->clone();
            else
                array->mSize = new Constant(llvm::cast<llvm::Constant>(arraySize));
        }
    }

    state.addFunctionBlock(instruction, value);
    Pointer::InclusionBased *pointer = new Pointer::InclusionBased();
    pointer->addMemoryTarget(&instruction, &instruction);
    state.addFunctionVariable(instruction, pointer);
}

void
Interpreter::load(const llvm::LoadInst &instruction, State &state)
{
    // Find the pointer in the state.  If the pointer is not
    // available, do nothing.
    Value *variable = state.findVariable(*instruction.getPointerOperand());
    if (!variable)
        return;
    const Pointer::InclusionBased &pointer = dynamic_cast<const Pointer::InclusionBased&>(*variable);

    // Pointer found. Merge all possible values and store the result
    // into the state.
    Value *mergedValue = NULL;
    Pointer::PlaceTargetMap::const_iterator it = pointer.getTargets().begin();
    for (; it != pointer.getTargets().end(); ++it)
    {
        Value *value = it->second.dereference(state);
        if (!value)
            continue;

        if (mergedValue)
            mergedValue->merge(*value);
        else
            mergedValue = value->clone();
    }

    if (!mergedValue)
        return;

    state.addFunctionVariable(instruction, mergedValue);
}

void
Interpreter::store(const llvm::StoreInst &instruction, State &state)
{
    // Find the pointer in the state.  If the pointer is not
    // available, do nothing.
    Value *variable = state.findVariable(*instruction.getPointerOperand());
    if (!variable)
        return;
    const Pointer::InclusionBased &pointer = dynamic_cast<const Pointer::InclusionBased&>(*variable);

    // Find the variable in the state.  Merge the provided value into
    // all targets.
    Value *value = state.findVariable(*instruction.getValueOperand());
    bool deleteValue = false;
    if (!value)
    {
        // Handle constants.
        if (llvm::isa<llvm::Constant>(instruction.getValueOperand()))
        {
            value = new Constant(llvm::cast<llvm::Constant>(instruction.getValueOperand()));
            deleteValue = true;
        }
        else
            return;
    }

    Pointer::PlaceTargetMap::const_iterator it = pointer.getTargets().begin();
    for (; it != pointer.getTargets().end(); ++it)
    {
        Value *dest = it->second.dereference(state);
        if (!dest)
            continue;

        dest->merge(*value);
    }

    if (deleteValue)
        delete value;
}

void
Interpreter::fence(const llvm::FenceInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::cmpxchg(const llvm::AtomicCmpXchgInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::atomicrmw(const llvm::AtomicRMWInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::getelementptr(const llvm::GetElementPtrInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::trunc(const llvm::TruncInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::zext(const llvm::ZExtInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::sext(const llvm::SExtInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fptrunc(const llvm::FPTruncInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fpext(const llvm::FPExtInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fptoui(const llvm::FPToUIInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fptosi(const llvm::FPToSIInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::uitofp(const llvm::UIToFpInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::sitofp(const llvm::SIToFPInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::ptrtoint(const llvm::PtrToIntInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::inttoptr(const llvm::IntToPtrInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::bitcast(const llvm::BitCastInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::icmp(const llvm::ICmpInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fcmp(const llvm::FCmpInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::phi(const llvm::PHINode &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::select(const llvm::SelectInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::call(const llvm::CallInst &instruction, State &state)
{
    interpretCall(*this, instruction, state);
}

void
Interpreter::va_arg(const llvm::VAArgInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::landingpad(const llvm::LandingPadInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
