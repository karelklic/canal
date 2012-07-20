#include "Interpreter.h"
#include "State.h"
#include "Utils.h"
#include "Value.h"
#include "IntegerContainer.h"
#include "IntegerBits.h"
#include "Pointer.h"
#include "ArraySingleItem.h"
#include "ArrayExactLimitedSize.h"
#include "FloatRange.h"
#include "Constant.h"
#include "Stack.h"
#include "Structure.h"
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
#if LLVM_MAJOR >= 3
        // Resume instruction is available since LLVM 3.0
        else if (llvm::isa<llvm::ResumeInst>(instruction))
            resume((const llvm::ResumeInst&)instruction, state);
#endif
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
	case llvm::Instruction::Add:  add(binaryOp, stack);  break;
	case llvm::Instruction::FAdd: fadd(binaryOp, stack); break;
	case llvm::Instruction::Sub:  sub(binaryOp, stack);  break;
	case llvm::Instruction::FSub: fsub(binaryOp, stack); break;
	case llvm::Instruction::Mul:  mul(binaryOp, stack);  break;
	case llvm::Instruction::FMul: fmul(binaryOp, stack); break;
	case llvm::Instruction::UDiv: udiv(binaryOp, stack); break;
	case llvm::Instruction::SDiv: sdiv(binaryOp, stack); break;
	case llvm::Instruction::FDiv: fdiv(binaryOp, stack); break;
	case llvm::Instruction::URem: urem(binaryOp, stack); break;
	case llvm::Instruction::SRem: srem(binaryOp, stack); break;
	case llvm::Instruction::FRem: frem(binaryOp, stack); break;
	case llvm::Instruction::Shl:  shl(binaryOp, stack);  break;
	case llvm::Instruction::LShr: lshr(binaryOp, stack); break;
	case llvm::Instruction::AShr: ashr(binaryOp, stack); break;
	case llvm::Instruction::And:  and_(binaryOp, stack); break;
	case llvm::Instruction::Or:   or_(binaryOp, stack);  break;
	case llvm::Instruction::Xor:  xor_(binaryOp, stack); break;
	default:
            CANAL_FATAL_ERROR(binaryOp);
	}
    }
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
    else
        CANAL_FATAL_ERROR("unknown instruction: " << instruction.getOpcodeName());
}

void
Interpreter::ret(const llvm::ReturnInst &instruction, State &state)
{
    llvm::Value *value = instruction.getReturnValue();
    // Return value is optional, some functions return nothing.
    if (!value)
        return;

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

// Given a place in source code, return the corresponding variable
// from the abstract interpreter state. If the place contains a
// constant, fill the provided constant variable with it.
// @return
//  Returns a pointer to the variable if it is found in the state.
//  Returns a pointer to the provided constant if the place contains a
//  constant.  Otherwise, it returns NULL.
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
getFloatingPointSemantics(const llvm::Type &type)
{
    CANAL_ASSERT(type.isFloatingPointTy());

    const llvm::fltSemantics *semantics;
#if (LLVM_MAJOR == 3 && LLVM_MINOR >= 1) || LLVM_MAJOR > 3
    if (type.isHalfTy())
        semantics = &llvm::APFloat::IEEEhalf;
    else
#endif
    if (type.isFloatTy())
        semantics = &llvm::APFloat::IEEEsingle;
    else if (type.isDoubleTy())
        semantics = &llvm::APFloat::IEEEdouble;
    else if (type.isFP128Ty())
        semantics = &llvm::APFloat::IEEEquad;
    else if (type.isPPC_FP128Ty())
        semantics = &llvm::APFloat::PPCDoubleDouble;
    else if (type.isX86_FP80Ty())
        semantics = &llvm::APFloat::x87DoubleExtended;
    else
        CANAL_DIE();
    return semantics;
}


static Value *
typeToEmptyValue(const llvm::Type &type, const llvm::Module &module)
{
    if (type.isVoidTy())
        return NULL;

    if (type.isIntegerTy())
    {
        llvm::IntegerType &integerType = llvm::cast<llvm::IntegerType>(type);
        return new Integer::Container(integerType.getBitWidth());
    }

    if (type.isFloatingPointTy())
    {
        const llvm::fltSemantics *semantics = getFloatingPointSemantics(type);
        return new Float::Range(*semantics);
    }

    if (type.isPointerTy())
        return new Pointer::InclusionBased(module);

    if (type.isArrayTy() || type.isVectorTy())
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (type.isStructTy())
    {
        const llvm::StructType &structType = llvm::cast<llvm::StructType>(type);
        Structure *structure = new Structure();

        llvm::StructType::element_iterator it = structType.element_begin(),
            itend = structType.element_end();
        for (; it != itend; ++it)
            structure->mMembers.push_back(typeToEmptyValue(**it, module));

        return structure;
    }

    CANAL_DIE_MSG("unsupported llvm::Type::TypeID: " << type.getTypeID());
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
        printf("Function \"%s\" not available.\n", function->getName().data());

        // TODO: Set memory accessed by non-static globals to
        // the Top value.

        // Create result TOP value of required type.
        const llvm::Type *type = instruction.getType();
        Value *returnedValue = typeToEmptyValue(*instruction.getType(), stack.getModule());

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

#if LLVM_MAJOR >= 3
// Resume instruction is available since LLVM 3.0
void
Interpreter::resume(const llvm::ResumeInst &instruction, State &state)
{
    CANAL_NOT_IMPLEMENTED();
}
#endif

void
Interpreter::unreachable(const llvm::UnreachableInst &instruction, State &state)
{
    // Ignore.
}

static void
binaryOperation(const llvm::BinaryOperator &instruction,
                Stack &stack,
                void(Value::*operation)(const Value&, const Value&))
{
    State &state = stack.getCurrentState();

    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.
    Constant constants[2];
    Value *values[2] = {
        variableOrConstant(*instruction.getOperand(0), state, constants[0]),
        variableOrConstant(*instruction.getOperand(1), state, constants[1])
    };
    if (!values[0] || !values[1])
        return;

    // Create result value of required type and then run the desired
    // operation.
    Value *result = typeToEmptyValue(*instruction.getType(), stack.getModule());
    ((result)->*(operation))(*values[0], *values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void Interpreter::add(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::add);
}

void
Interpreter::fadd(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::fadd);
}

void
Interpreter::sub(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::sub);
}

void
Interpreter::fsub(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::fsub);
}

void
Interpreter::mul(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::mul);
}

void
Interpreter::fmul(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::fmul);
}

void
Interpreter::udiv(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::udiv);
}

void
Interpreter::sdiv(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::sdiv);
}

void
Interpreter::fdiv(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::fdiv);
}

void
Interpreter::urem(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::urem);
}

void
Interpreter::srem(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::srem);
}

void
Interpreter::frem(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::frem);
}

void
Interpreter::shl(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::shl);
}

void
Interpreter::lshr(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::lshr);
}

void
Interpreter::ashr(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::ashr);
}

void
Interpreter::and_(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::and_);
}

void
Interpreter::or_(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::or_);
}

void
Interpreter::xor_(const llvm::BinaryOperator &instruction, Stack &stack)
{
    binaryOperation(instruction, stack, &Value::xor_);
}

void
Interpreter::extractelement(const llvm::ExtractElementInst &instruction, State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.  Fixpoint
    // calculation is probably not far enough.
    Constant constants[2];
    Value *values[2] = {
        variableOrConstant(*instruction.getOperand(0), state, constants[0]),
        variableOrConstant(*instruction.getOperand(1), state, constants[1])
    };
    if (!values[0] || !values[1])
        return;

    const Array::ExactLimitedSize *array =
        dynamic_cast<const Array::ExactLimitedSize*>(values[0]);

    if (const Constant *constant = dynamic_cast<const Constant*>(values[0]))
    {
        Value *modifiable = constant->toModifiableValue();
        array = dynamic_cast<const Array::ExactLimitedSize*>(modifiable);
    }

    CANAL_ASSERT_MSG(array, "Invalid type of array.");
    Value *result = array->getValue(*values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void
Interpreter::insertelement(const llvm::InsertElementInst &instruction, State &state)
{
    // Find operands in state, and encapsulate constant operands (such
    // as numbers).  If some operand is not known, exit.  Fixpoint
    // calculation is probably not far enough.
    Constant constants[3];
    Value *values[3] = {
        variableOrConstant(*instruction.getOperand(0), state, constants[0]),
        variableOrConstant(*instruction.getOperand(1), state, constants[1]),
        variableOrConstant(*instruction.getOperand(2), state, constants[2])
    };
    if (!values[0] || !values[1] || !values[2])
        return;

    Value *result = values[0]->clone();

    Array::ExactLimitedSize *resultAsArray =
        dynamic_cast<Array::ExactLimitedSize*>(result);

    if (Constant *constant = dynamic_cast<Constant*>(result))
    {
        Value *modifiable = constant->toModifiableValue();
        result = resultAsArray =
            dynamic_cast<Array::ExactLimitedSize*>(modifiable);
    }

    CANAL_ASSERT_MSG(result, "Invalid type of array.");
    resultAsArray->set(*values[2], *values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
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
    const llvm::Type *type = instruction.getAllocatedType();
    Value *value = typeToEmptyValue(*type, stack.getModule());

    if (instruction.isArrayAllocation())
    {
        Array::SingleItem *array = new Array::SingleItem();
        array->mValue = value;
        value = array;

        // Here is a bug, maybe. Array size might be unavailable
        // because of fixpoint calculation.
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
    Pointer::InclusionBased *pointer =
        new Pointer::InclusionBased(stack.getModule());

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

    const Pointer::InclusionBased &pointer =
        dynamic_cast<const Pointer::InclusionBased&>(*variable);

    // Pointer found. Merge all possible values and store the result
    // into the state.
    Value *mergedValue = NULL;
    Pointer::PlaceTargetMap::const_iterator it = pointer.mTargets.begin();
    for (; it != pointer.mTargets.end(); ++it)
    {
        std::vector<Value*> values = it->second.dereference(state);
        std::vector<Value*>::const_iterator it = values.begin();
        for (; it != values.end(); ++it)
        {
            if (mergedValue)
                mergedValue->merge(**it);
            else
                mergedValue = (*it)->clone();
        }
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

    const Pointer::InclusionBased &pointer =
        dynamic_cast<const Pointer::InclusionBased&>(*variable);

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
    Pointer::PlaceTargetMap::const_iterator it = pointer.mTargets.begin();
    for (; it != pointer.mTargets.end(); ++it)
    {
        std::vector<Value*> destinations = it->second.dereference(state);
        std::vector<Value*>::iterator it = destinations.begin();
        for (; it != destinations.end(); ++it)
            (*it)->merge(*value);
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
Interpreter::getelementptr(const llvm::GetElementPtrInst &instruction,
                           Stack &stack)
{
    CANAL_ASSERT(instruction.getNumOperands() > 1);
    State &state = stack.getCurrentState();

    // Find the base pointer.
    Value *base = state.findVariable(*instruction.getPointerOperand());
    if (!base)
        return;

    Pointer::InclusionBased *source =
        dynamic_cast<Pointer::InclusionBased*>(base);
    CANAL_ASSERT(source);

    Pointer::InclusionBased *result = source->clone();

    // We get offsets. Either constants or Integer::Container.
    // Pointer points either to an array (or array offset), or to a
    // struct (or struct member).  Pointer might have multiple
    // targets.

    llvm::GetElementPtrInst::const_op_iterator it = instruction.idx_begin(),
        itend = instruction.idx_end();
    for (; it != itend; ++it)
    {
        if (llvm::isa<llvm::ConstantInt>(it))
        {
            llvm::ConstantInt *constant = llvm::cast<llvm::ConstantInt>(it);
            Pointer::PlaceTargetMap::iterator it = result->mTargets.begin();
            for (; it != result->mTargets.end(); ++it)
                it->second.mOffsets.push_back(new Constant(constant));
        }
        else
        {
            Value *offset = state.findVariable(*it->get());
            if (!offset)
            {
                // There should be a return as not all offsets are
                // necesarily known at each pass before reaching a
                // fixpoint, but we need to implement a check ensuring
                // that all instructions are evaluated before reaching
                // fixpoint.
                CANAL_DIE();
                return;
            }

            Integer::Container &integer = dynamic_cast<Integer::Container&>(*offset);
            Pointer::PlaceTargetMap::iterator it = result->mTargets.begin();
            for (; it != result->mTargets.end(); ++it)
                it->second.mOffsets.push_back(integer.clone());
        }

    }

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
Interpreter::uitofp(const llvm::UIToFPInst &instruction, State &state)
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
    const llvm::Type *source = instruction.getSrcTy();
    const llvm::Type *destination = instruction.getDestTy();

    CANAL_ASSERT_MSG(source->isPointerTy() && destination->isPointerTy(),
                     "Bitcast for non-pointers is not implemented.");

    Value *operand = state.findVariable(*instruction.getOperand(0));
    if (!operand)
        return;

    Value *resultValue = operand->clone();

     Pointer::InclusionBased &pointer =
        dynamic_cast<Pointer::InclusionBased&>(*resultValue);

    pointer.mBitcastFrom = instruction.getSrcTy();
    pointer.mBitcastTo = instruction.getDestTy();

    state.addFunctionVariable(instruction, resultValue);
}

typedef void(Value::*CmpOperation)(const Value&,
                                   const Value&,
                                   llvm::CmpInst::Predicate predicate);

static void
cmpOperation(const llvm::CmpInst &instruction,
             State &state,
             CmpOperation cmpOperation)
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
    ((resultValue)->*(cmpOperation))(*values[0],
                                     *values[1],
                                     instruction.getPredicate());

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
