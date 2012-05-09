#include "Interpreter.h"
#include "State.h"
#include "Utils.h"
#include "Value.h"
#include "Integer.h"
#include "Pointer.h"
#include "Array.h"
#include "Float.h"
#include "Constant.h"
#include "Stack.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <llvm/Support/CFG.h>
#include <map>
#include <cassert>
#include <cstdio>

namespace Canal {

bool
Interpreter::step(Stack &stack)
{
    interpretInstruction(stack);
    return stack.nextInstruction();
}

void
Interpreter::interpretInstruction(Stack &stack)
{
    const llvm::Instruction &instruction = stack.getCurrentInstruction();
    State &state = stack.getCurrentState();

    if (llvm::isa<llvm::AllocaInst>(instruction))
        alloca_((const llvm::AllocaInst&)instruction, stack);
    else if (llvm::isa<llvm::StoreInst>(instruction))
        store((const llvm::StoreInst&)instruction, state);
    else if (llvm::isa<llvm::CallInst>(instruction))
        call((const llvm::CallInst&)instruction, stack);
    else if (llvm::isa<llvm::LoadInst>(instruction))
        load((const llvm::LoadInst&)instruction, state);
    else if (llvm::isa<llvm::ICmpInst>(instruction))
        icmp((const llvm::ICmpInst&)instruction, state);
    else if (llvm::isa<llvm::FCmpInst>(instruction))
        fcmp((const llvm::FCmpInst&)instruction, state);
    else if (llvm::isa<llvm::GetElementPtrInst>(instruction))
        getelementptr((const llvm::GetElementPtrInst&)instruction, stack);
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
            invoke((const llvm::InvokeInst&)instruction, stack);
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
        CANAL_DIE_MSG("unknown instruction: " << instruction.getOpcodeName());
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

static const llvm::fltSemantics *
getFloatingPointSemantics(llvm::Type *type)
{
    CANAL_ASSERT(type->isFloatingPointTy());

    const llvm::fltSemantics *semantics;
#if (LLVM_MAJOR == 3 && LLVM_MINOR >= 1) || LLVM_MAJOR > 3
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
    return semantics;
}

template<typename T> static void
interpretCall(const T &instruction, Stack &stack)
{
    State &state = stack.getCurrentState();
    llvm::Function *function = instruction.getCalledFunction();
    // TODO: Handle some intristic functions.  Some of them can be
    // safely ignored.
    if (!function || function->isIntrinsic() || function->isDeclaration())
    {
        // Function not found.  Set the resultant value to the Top
        // value.

        // TODO: Set memory accessed by non-static globals to
        // the Top value.

        // Create result TOP value of required type.
        llvm::Type *type = instruction.getType();
        Value *returnedValue = NULL;
        if (type->isVoidTy())
            returnedValue = NULL;
        else if (type->isIntegerTy())
        {
            llvm::IntegerType *integerType = llvm::cast<llvm::IntegerType>(type);
            returnedValue = new Integer::Container(integerType->getBitWidth());
        }
        else if (type->isFloatingPointTy())
        {
            const llvm::fltSemantics *semantics = getFloatingPointSemantics(type);
            returnedValue = new Float::Range(*semantics);
        }
        else if (type->isPointerTy())
        {
            returnedValue = new Pointer::InclusionBased(stack.getModule());
        }
        else
            CANAL_DIE_MSG("unsupported llvm::Type::TypeID: " << type->getTypeID());

        // If the function returns nothing (void), we are finished.
        if (!returnedValue)
            return;

        AccuracyValue *accuracyValue = dynamic_cast<AccuracyValue*>(returnedValue);
        if (accuracyValue)
            accuracyValue->setTop();

        state.addFunctionVariable(instruction, returnedValue);
        return;
    }

    State initialState(state);
    initialState.clearFunctionLevel();
    llvm::Function::ArgumentListType::const_iterator it = function->getArgumentList().begin();
    for (int i = 0; i < instruction.getNumArgOperands(); ++i, ++it)
    {
        llvm::Value *operand = instruction.getArgOperand(i);
        Constant c;
        Value *value = variableOrConstant(*operand, state, c);
        if (!value)
            return;
        initialState.addFunctionVariable(*it, value->clone());
    }

    stack.addFrame(*function, initialState);
}

void
Interpreter::invoke(const llvm::InvokeInst &instruction, Stack &stack)
{
    interpretCall(instruction, stack);
}

void
Interpreter::resume(const llvm::ResumeInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::unreachable(const llvm::UnreachableInst &instruction, State &state)
{
    // Ignore.
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
        const llvm::fltSemantics *semantics = getFloatingPointSemantics(type);
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
Interpreter::alloca_(const llvm::AllocaInst &instruction, Stack &stack)
{
    State &state = stack.getCurrentState();
    llvm::Type *type = instruction.getAllocatedType();
    Value *value = NULL;
    if (type->isIntegerTy())
    {
        llvm::IntegerType &integerType = llvm::cast<llvm::IntegerType>(*type);
        value = new Integer::Container(integerType.getBitWidth());
    }
    else if (type->isPointerTy())
        value = new Pointer::InclusionBased(stack.getModule());
    else
        CANAL_DIE();

    if (instruction.isArrayAllocation())
    {
        Array::SingleItem *array = new Array::SingleItem();
        array->mValue = value;
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
    Pointer::InclusionBased *pointer = new Pointer::InclusionBased(stack.getModule());
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
        // Handle storing of constant values.
        if (llvm::isa<llvm::Constant>(instruction.getValueOperand()))
        {
            value = new Constant(llvm::cast<llvm::Constant>(instruction.getValueOperand()));
            deleteValue = true;
        }
        else
            return;
    }

    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
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
Interpreter::getelementptr(const llvm::GetElementPtrInst &instruction, Stack &stack)
{
    CANAL_ASSERT(instruction.getNumOperands() > 1);
    State &state = stack.getCurrentState();

    // Find the base pointer.
    Value *pointer = state.findVariable(*instruction.getOperand(0));
    if (!pointer)
        return;

    // TODO: add proper target
    CANAL_NOT_IMPLEMENTED();
    Pointer::InclusionBased *result = new Pointer::InclusionBased(stack.getModule());
    state.addFunctionVariable(instruction, result);
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

static void
cmpOperation(const llvm::CmpInst &instruction, State &state, void(Value::*cmpOperation)(const Value&, const Value&, llvm::CmpInst::Predicate predicate))
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

    // TODO: suppot arrays
    Value *resultValue = new Integer::Container(1);
    ((resultValue)->*(cmpOperation))(*values[0], *values[1], instruction.getPredicate());
    state.addFunctionVariable(instruction, resultValue);
}

void
Interpreter::icmp(const llvm::ICmpInst &instruction, State &state)
{
    cmpOperation(instruction, state, &Value::icmp);
}

void
Interpreter::fcmp(const llvm::FCmpInst &instruction, State &state)
{
    cmpOperation(instruction, state, &Value::fcmp);
}

void
Interpreter::phi(const llvm::PHINode &instruction, State &state)
{
    Value *mergedValue = NULL;
    for (int i = 0; i < instruction.getNumIncomingValues(); ++i)
    {
        Constant c;
        Value *value = variableOrConstant(*instruction.getIncomingValue(i), state, c);
        if (!value)
            continue;
        if (mergedValue)
            mergedValue->merge(*value);
        else
        {
            Constant *constant = dynamic_cast<Constant*>(value);
            if (constant)
                mergedValue = constant->toModifiableValue();
            else
                mergedValue = value->clone();
        }
    }

    state.addFunctionVariable(instruction, mergedValue);
}

void
Interpreter::select(const llvm::SelectInst &instruction, State &state)
{
    Value *condition = state.findVariable(*instruction.getCondition());
    if (!condition)
        return;

    Constant trueConstant, falseConstant;
    Value *trueValue = variableOrConstant(*instruction.getTrueValue(), state, trueConstant);
    Value *falseValue = variableOrConstant(*instruction.getFalseValue(), state, falseConstant);

    Value *resultValue;
    const Integer::Container &conditionInt = dynamic_cast<const Integer::Container&>(*condition);
    CANAL_ASSERT(conditionInt.getBits().getBitWidth() == 1);
    switch (conditionInt.getBits().getBitValue(0))
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
        resultValue->merge(*falseValue);
        break;
    default:
        CANAL_DIE();
    }

    state.addFunctionVariable(instruction, resultValue);
}

void
Interpreter::call(const llvm::CallInst &instruction, Stack &stack)
{
    interpretCall(instruction, stack);
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
