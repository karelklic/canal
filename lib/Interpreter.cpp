#include "Interpreter.h"
#include "ArrayExactSize.h"
#include "ArraySingleItem.h"
#include "Constant.h"
#include "Environment.h"
#include "FloatRange.h"
#include "IntegerBits.h"
#include "IntegerContainer.h"
#include "Pointer.h"
#include "Stack.h"
#include "State.h"
#include "Structure.h"
#include "Utils.h"
#include "Value.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <llvm/Support/CFG.h>
#include <map>
#include <cassert>
#include <cstdio>

namespace Canal {

bool
Interpreter::step(Stack &stack,
                  const Environment &environment)
{
    interpretInstruction(stack, environment);
    return stack.nextInstruction();
}

void
Interpreter::interpretInstruction(Stack &stack,
                                  const Environment &environment)
{
    const llvm::Instruction &instruction =
        stack.getCurrentInstruction();
    State &state = stack.getCurrentState();

    if (llvm::isa<llvm::CallInst>(instruction))
        call((const llvm::CallInst&)instruction, stack, environment);
    else if (llvm::isa<llvm::ICmpInst>(instruction))
        icmp((const llvm::ICmpInst&)instruction, state, environment);
    else if (llvm::isa<llvm::FCmpInst>(instruction))
        fcmp((const llvm::FCmpInst&)instruction, state, environment);
    else if (llvm::isa<llvm::ExtractElementInst>(instruction))
        extractelement((const llvm::ExtractElementInst&)instruction, state, environment);
    else if (llvm::isa<llvm::GetElementPtrInst>(instruction))
        getelementptr((const llvm::GetElementPtrInst&)instruction, stack, environment);
    else if (llvm::isa<llvm::InsertElementInst>(instruction))
        insertelement((const llvm::InsertElementInst&)instruction, state, environment);
    else if (llvm::isa<llvm::InsertValueInst>(instruction))
        insertvalue((const llvm::InsertValueInst&)instruction, state, environment);
#if LLVM_MAJOR >= 3
    // Instructions available since LLVM 3.0
    else if (llvm::isa<llvm::LandingPadInst>(instruction))
        landingpad((const llvm::LandingPadInst&)instruction, state, environment);
    else if (llvm::isa<llvm::AtomicCmpXchgInst>(instruction))
        cmpxchg((const llvm::AtomicCmpXchgInst&)instruction, state, environment);
    else if (llvm::isa<llvm::AtomicRMWInst>(instruction))
        atomicrmw((const llvm::AtomicRMWInst&)instruction, state, environment);
    else if (llvm::isa<llvm::FenceInst>(instruction))
        fence((const llvm::FenceInst&)instruction, state, environment);
#endif
    else if (llvm::isa<llvm::PHINode>(instruction))
        phi((const llvm::PHINode&)instruction, state, environment);
    else if (llvm::isa<llvm::SelectInst>(instruction))
        select((const llvm::SelectInst&)instruction, state, environment);
    else if (llvm::isa<llvm::ShuffleVectorInst>(instruction))
        shufflevector((const llvm::ShuffleVectorInst&)instruction, stack, environment);
    else if (llvm::isa<llvm::StoreInst>(instruction))
        store((const llvm::StoreInst&)instruction, state, environment);
    else if (llvm::isa<llvm::UnaryInstruction>(instruction))
    {
        if (llvm::isa<llvm::AllocaInst>(instruction))
            alloca_((const llvm::AllocaInst&)instruction, stack, environment);
        else if (llvm::isa<llvm::CastInst>(instruction))
        {
            if (llvm::isa<llvm::BitCastInst>(instruction))
                bitcast((const llvm::BitCastInst&)instruction, state, environment);
            else if (llvm::isa<llvm::FPExtInst>(instruction))
                fpext((const llvm::FPExtInst&)instruction, state, environment);
            else if (llvm::isa<llvm::FPToSIInst>(instruction))
                fptosi((const llvm::FPToSIInst&)instruction, state, environment);
            else if (llvm::isa<llvm::FPToUIInst>(instruction))
                fptoui((const llvm::FPToUIInst&)instruction, state, environment);
            else if (llvm::isa<llvm::FPTruncInst>(instruction))
                fptrunc((const llvm::FPTruncInst&)instruction, state, environment);
            else if (llvm::isa<llvm::IntToPtrInst>(instruction))
                inttoptr((const llvm::IntToPtrInst&)instruction, state, environment);
            else if (llvm::isa<llvm::PtrToIntInst>(instruction))
                ptrtoint((const llvm::PtrToIntInst&)instruction, state, environment);
            else if (llvm::isa<llvm::SExtInst>(instruction))
                sext((const llvm::SExtInst&)instruction, state, environment);
            else if (llvm::isa<llvm::SIToFPInst>(instruction))
                sitofp((const llvm::SIToFPInst&)instruction, state, environment);
            else if (llvm::isa<llvm::TruncInst>(instruction))
                trunc((const llvm::TruncInst&)instruction, state, environment);
            else if (llvm::isa<llvm::UIToFPInst>(instruction))
                uitofp((const llvm::UIToFPInst&)instruction, state, environment);
            else if (llvm::isa<llvm::ZExtInst>(instruction))
                zext((const llvm::ZExtInst&)instruction, state, environment);
            else
                CANAL_FATAL_ERROR("Unknown cast instruction: " << instruction);
        }
        else if (llvm::isa<llvm::ExtractValueInst>(instruction))
            extractvalue((const llvm::ExtractValueInst&)instruction, state, environment);
        else if (llvm::isa<llvm::LoadInst>(instruction))
            load((const llvm::LoadInst&)instruction, state, environment);
        else if (llvm::isa<llvm::VAArgInst>(instruction))
            va_arg((const llvm::VAArgInst&)instruction, state, environment);
        else
            CANAL_FATAL_ERROR("Unknown unary instruction: " << instruction);
    }
    else if (llvm::isa<llvm::TerminatorInst>(instruction))
    {
        if (llvm::isa<llvm::BranchInst>(instruction))
            br((const llvm::BranchInst&)instruction, state, environment);
        else if (llvm::isa<llvm::IndirectBrInst>(instruction))
            indirectbr((const llvm::IndirectBrInst&)instruction, state, environment);
        else if (llvm::isa<llvm::InvokeInst>(instruction))
            invoke((const llvm::InvokeInst&)instruction, stack, environment);
#if LLVM_MAJOR >= 3
        // Resume instruction is available since LLVM 3.0
        else if (llvm::isa<llvm::ResumeInst>(instruction))
            resume((const llvm::ResumeInst&)instruction, state, environment);
#endif
        else if (llvm::isa<llvm::ReturnInst>(instruction))
            ret((const llvm::ReturnInst&)instruction, state, environment);
        else if (llvm::isa<llvm::SwitchInst>(instruction))
            switch_((const llvm::SwitchInst&)instruction, state, environment);
        else if (llvm::isa<llvm::UnreachableInst>(instruction))
            unreachable((const llvm::UnreachableInst&)instruction, state, environment);
        else
            CANAL_FATAL_ERROR("Unknown terminator instruction: " << instruction);
    }
    else if (llvm::isa<llvm::BinaryOperator>(instruction))
    {
        const llvm::BinaryOperator &binaryOp = (const llvm::BinaryOperator&)instruction;
        assert(binaryOp.getNumOperands() == 2);
        switch (binaryOp.getOpcode())
	{
	case llvm::Instruction::Add:  add(binaryOp, state, environment);  break;
	case llvm::Instruction::FAdd: fadd(binaryOp, state, environment); break;
	case llvm::Instruction::Sub:  sub(binaryOp, state, environment);  break;
	case llvm::Instruction::FSub: fsub(binaryOp, state, environment); break;
	case llvm::Instruction::Mul:  mul(binaryOp, state, environment);  break;
	case llvm::Instruction::FMul: fmul(binaryOp, state, environment); break;
	case llvm::Instruction::UDiv: udiv(binaryOp, state, environment); break;
	case llvm::Instruction::SDiv: sdiv(binaryOp, state, environment); break;
	case llvm::Instruction::FDiv: fdiv(binaryOp, state, environment); break;
	case llvm::Instruction::URem: urem(binaryOp, state, environment); break;
	case llvm::Instruction::SRem: srem(binaryOp, state, environment); break;
	case llvm::Instruction::FRem: frem(binaryOp, state, environment); break;
	case llvm::Instruction::Shl:  shl(binaryOp, state, environment);  break;
	case llvm::Instruction::LShr: lshr(binaryOp, state, environment); break;
	case llvm::Instruction::AShr: ashr(binaryOp, state, environment); break;
	case llvm::Instruction::And:  and_(binaryOp, state, environment); break;
	case llvm::Instruction::Or:   or_(binaryOp, state, environment);  break;
	case llvm::Instruction::Xor:  xor_(binaryOp, state, environment); break;
	default:
            CANAL_FATAL_ERROR(binaryOp);
	}
    }
    else
        CANAL_FATAL_ERROR("Unknown instruction: " << instruction.getOpcodeName());
}

void
Interpreter::ret(const llvm::ReturnInst &instruction,
                 State &state,
                 const Environment &environment)
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

            Constant c(llvmCast<llvm::Constant>(value));
            state.mReturnedValue->merge(c);
        }
        else
            state.mReturnedValue = new Constant(llvmCast<llvm::Constant>(value));
    }
}

void
Interpreter::br(const llvm::BranchInst &instruction,
                State &state,
                const Environment &environment)
{
    // Ignore.
}

void
Interpreter::switch_(const llvm::SwitchInst &instruction,
                     State &state,
                     const Environment &environment)
{
    // Ignore.
}

void
Interpreter::indirectbr(const llvm::IndirectBrInst &instruction,
                        State &state,
                        const Environment &environment)
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
        constant.mConstant = llvmCast<llvm::Constant>(&place);
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
        llvm::IntegerType &integerType = llvmCast<llvm::IntegerType>(type);
        return new Integer::Container(integerType.getBitWidth());
    }

    if (type.isFloatingPointTy())
    {
        const llvm::fltSemantics *semantics = getFloatingPointSemantics(type);
        return new Float::Range(*semantics);
    }

    if (type.isPointerTy())
    {
        const llvm::PointerType &pointerType = llvmCast<const llvm::PointerType>(type);
        return new Pointer::InclusionBased(module, pointerType.getElementType());
    }

    if (type.isArrayTy() || type.isVectorTy())
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (type.isStructTy())
    {
        const llvm::StructType &structType = llvmCast<llvm::StructType>(type);
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
interpretCall(const T &instruction,
              Stack &stack,
              const llvm::Module &module)
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
        Value *returnedValue = typeToEmptyValue(*instruction.getType(),
                                                module);

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
Interpreter::invoke(const llvm::InvokeInst &instruction,
                    Stack &stack,
                    const Environment &environment)
{
    interpretCall(instruction, stack, environment.mModule);
}

void
Interpreter::unreachable(const llvm::UnreachableInst &instruction,
                         State &state,
                         const Environment &environment)
{
    // Ignore.
}

static void
binaryOperation(const llvm::BinaryOperator &instruction,
                State &state,
                const llvm::Module &module,
                void(Value::*operation)(const Value&, const Value&))
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

    // Create result value of required type and then run the desired
    // operation.
    Value *result = typeToEmptyValue(*instruction.getType(), module);
    ((result)->*(operation))(*values[0], *values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void Interpreter::add(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::add);
}

void
Interpreter::fadd(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::fadd);
}

void
Interpreter::sub(const llvm::BinaryOperator &instruction,
                 State &state,
                 const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::sub);
}

void
Interpreter::fsub(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::fsub);
}

void
Interpreter::mul(const llvm::BinaryOperator &instruction,
                 State &state,
                 const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::mul);
}

void
Interpreter::fmul(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::fmul);
}

void
Interpreter::udiv(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::udiv);
}

void
Interpreter::sdiv(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::sdiv);
}

void
Interpreter::fdiv(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::fdiv);
}

void
Interpreter::urem(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::urem);
}

void
Interpreter::srem(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::srem);
}

void
Interpreter::frem(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::frem);
}

void
Interpreter::shl(const llvm::BinaryOperator &instruction,
                 State &state,
                 const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::shl);
}

void
Interpreter::lshr(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::lshr);
}

void
Interpreter::ashr(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::ashr);
}

void
Interpreter::and_(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::and_);
}

void
Interpreter::or_(const llvm::BinaryOperator &instruction,
                 State &state,
                 const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::or_);
}

void
Interpreter::xor_(const llvm::BinaryOperator &instruction,
                  State &state,
                  const Environment &environment)
{
    binaryOperation(instruction, state, environment.mModule, &Value::xor_);
}

void
Interpreter::extractelement(const llvm::ExtractElementInst &instruction,
                            State &state,
                            const Environment &environment)
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

    const Array::ExactSize *array =
        dynamic_cast<const Array::ExactSize*>(values[0]);

    if (const Constant *constant = dynamic_cast<const Constant*>(values[0]))
    {
        Value *modifiable = constant->toModifiableValue();
        array = dynamic_cast<const Array::ExactSize*>(modifiable);
    }

    CANAL_ASSERT_MSG(array, "Invalid type of array.");
    Value *result = array->getValue(*values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void
Interpreter::insertelement(const llvm::InsertElementInst &instruction,
                           State &state,
                           const Environment &environment)
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

    Array::ExactSize *resultAsArray =
        dynamic_cast<Array::ExactSize*>(result);

    if (Constant *constant = dynamic_cast<Constant*>(result))
    {
        Value *modifiable = constant->toModifiableValue();
        result = resultAsArray =
            dynamic_cast<Array::ExactSize*>(modifiable);
    }

    CANAL_ASSERT_MSG(result, "Invalid type of array.");
    resultAsArray->setItem(*values[2], *values[1]);

    // Store the result value to the state.
    state.addFunctionVariable(instruction, result);
}

void
Interpreter::shufflevector(const llvm::ShuffleVectorInst &instruction,
                           Stack &stack,
                           const Environment &environment)
{
    State &state = stack.getCurrentState();

    Constant constants[2];
    Value *values[2] = {
        variableOrConstant(*instruction.getOperand(0), state, constants[0]),
        variableOrConstant(*instruction.getOperand(1), state, constants[1])
    };
    if (!values[0] || !values[1])
        return;

    Array::ExactSize *array0 = dynamic_cast<Array::ExactSize*>(values[0]);
    CANAL_ASSERT_MSG(array0, "Invalid type in shufflevector.");
    Array::ExactSize *array1 = dynamic_cast<Array::ExactSize*>(values[1]);
    CANAL_ASSERT_MSG(array1, "Invalid type in shufflevector.");

    Array::ExactSize *result = new Array::ExactSize();

#if LLVM_MAJOR >= 3
    llvm::SmallVector<int, 16> shuffleMask = instruction.getShuffleMask();
#else
    llvm::SmallVector<int, 16> shuffleMask;
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
            int constantInt = llvm::isa<llvm::UndefValue>(constant) ? -1 :
                llvmCast<llvm::ConstantInt>(constant)->getZExtValue();
            shuffleMask.push_back(constantInt);
        }
    }
#endif

    llvm::SmallVector<int, 16>::iterator it = shuffleMask.begin(),
        itend = shuffleMask.end();
    for (; it != itend; ++it)
    {
        int offset = *it;
        if (offset == -1)
        {
            Value *value = typeToEmptyValue(*instruction.getType()->getElementType(),
                                            environment.mModule);

            result->mValues.push_back(value);
        }
        else if (offset < array0->size())
            result->mValues.push_back(array0->mValues[offset]->clone());
        else
        {
            CANAL_ASSERT_MSG(offset < array0->size() + array1->size(),
                             "Offset out of bounds.");
            offset -= array0->size();
            result->mValues.push_back(array1->mValues[offset]->clone());
        }
    }

    state.addFunctionVariable(instruction, result);
}

template <typename T> static Value *
getValueLocation(Value *aggregate, const T &instruction)
{
    Value *item = aggregate;
    typename T::idx_iterator it = instruction.idx_begin(),
        itend = instruction.idx_end();
    for (; it != itend; ++it)
    {
        if (const Constant *constant = dynamic_cast<const Constant *>(item))
        {
            // It would make sense to implement getValue() for
            // Constant.  Converting constant via getModifableValue()
            // leads to precision loss.
            CANAL_NOT_IMPLEMENTED();
        }

        const Array::Interface *array = dynamic_cast<const Array::Interface*>(item);
        CANAL_ASSERT_MSG(array, "ExtractValue reached an unsupported type.");
        item = array->getItem(*it);
    }

    return item;
}

void
Interpreter::extractvalue(const llvm::ExtractValueInst &instruction,
                          State &state,
                          const Environment &environment)
{
    Constant constant;
    Value *aggregate = variableOrConstant(
        *instruction.getAggregateOperand(),
        state,
        constant);

    if (!aggregate)
        return;

    Value *item = getValueLocation(aggregate, instruction);
    state.addFunctionVariable(instruction, item->clone());
}

void
Interpreter::insertvalue(const llvm::InsertValueInst &instruction,
                         State &state,
                         const Environment &environment)
{
    Constant aggregateConstant;
    Value *aggregate = variableOrConstant(
        *instruction.getAggregateOperand(),
        state,
        aggregateConstant);

    if (!aggregate)
        return;

    Constant insertedConstant;
    Value *insertedValue = variableOrConstant(
        *instruction.getInsertedValueOperand(),
        state,
        insertedConstant);

    if (!insertedValue)
        return;

    Value *result = aggregate->clone();
    Value *item = getValueLocation(result, instruction);
    item->merge(*insertedValue);
    state.addFunctionVariable(instruction, result);
}

void
Interpreter::alloca_(const llvm::AllocaInst &instruction,
                     Stack &stack,
                     const Environment &environment)
{
    State &state = stack.getCurrentState();
    const llvm::Type *type = instruction.getAllocatedType();
    Value *value = typeToEmptyValue(*type, environment.mModule);

    if (instruction.isArrayAllocation())
    {
        const llvm::Value *arraySize = instruction.getArraySize();
        Value *abstractSize = NULL;

        if (llvm::isa<llvm::ConstantInt>(arraySize))
        {
            const llvm::ConstantInt *constant =
                llvmCast<llvm::ConstantInt>(arraySize);

            abstractSize = new Constant(constant);
        }
        else
        {
            abstractSize = state.findVariable(*arraySize);
            // Size is not necesarily known at each pass before
            // reaching a fixpoint.
            if (!abstractSize)
            {
                delete value;
                return;
            }

            abstractSize = abstractSize->clone();
        }

        Array::SingleItem *array = new Array::SingleItem();
        array->mValue = value;
        array->mSize = abstractSize;
        value = array;
    }

    state.addFunctionBlock(instruction, value);
    Pointer::InclusionBased *pointer =
        new Pointer::InclusionBased(environment.mModule, type);

    pointer->addTarget(Pointer::Target::FunctionBlock,
                       &instruction,
                       &instruction,
                       std::vector<Value*>(),
                       NULL);

    state.addFunctionVariable(instruction, pointer);
}

void
Interpreter::load(const llvm::LoadInst &instruction,
                  State &state,
                  const Environment &environment)
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
    Value *mergedValue = pointer.dereferenceAndMerge(state);
    if (!mergedValue)
        return;

    state.addFunctionVariable(instruction, mergedValue);
}

// Went through the getelementptr offsets and assembly a list of
// abstract values representing these offsets.
// @returns
//   True if the the operation has been successful.  This means that
//   state contained all abstract values used as an offset.  False
//   otherwise.  If false is returned, the result vector is empty.
// @param result
//   Vector where all offsets are going to be stored as abstract
//   values.  Caller takes ownership of the values.
template<typename T> static bool
getElementPtrOffsets(std::vector<Value*> &result,
                     T iteratorStart,
                     T iteratorEnd,
                     const State &state)
{
    // Check that all variables exist before building the offset list.
    for (T it = iteratorStart; it != iteratorEnd; ++it)
    {
        if (llvm::isa<llvm::ConstantInt>(it))
            continue;

        Value *offset = state.findVariable(*it->get());
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
            llvm::ConstantInt *constant =
                llvmCast<llvm::ConstantInt>(it);

            result.push_back(new Constant(constant));
        }
        else
        {
            Value *offset = state.findVariable(*it->get());
            result.push_back(offset->clone());
        }
    }

    return true;
}

void
Interpreter::store(const llvm::StoreInst &instruction,
                   State &state,
                   const Environment &environment)
{
    // Find the pointer in the state.  If the pointer is not
    // available, do nothing.
    Value *variable = state.findVariable(*instruction.getPointerOperand());
    if (!variable)
        return;

    Pointer::InclusionBased &pointer =
        dynamic_cast<Pointer::InclusionBased&>(*variable);

    // Find the variable in the state.  Merge the provided value into
    // all targets.
    Value *value = state.findVariable(*instruction.getValueOperand());
    bool deleteValue = false;
    if (!value)
    {
        // Handle storing of constant values.
        const llvm::Constant *constant =
            llvm::dyn_cast<llvm::Constant>(instruction.getValueOperand());

        if (!constant)
        {
            // Give up.  Fixpoint calculation has not yet provided us
            // the variable.
            return;
        }

        deleteValue = true;

        const llvm::ConstantExpr *constantExpr =
            llvm::dyn_cast<llvm::ConstantExpr>(constant);

        // Handle storing of getelementptr constant.  Our abstract
        // pointer value does not handle that.
        if (constantExpr &&
            constantExpr->getOpcode() == llvm::Instruction::GetElementPtr)
        {
            std::vector<Value*> offsets;
            bool allOffsetsPresent = getElementPtrOffsets(
                offsets,
                constantExpr->op_begin() + 1,
                constantExpr->op_end(),
                state);

            CANAL_ASSERT_MSG(allOffsetsPresent,
                             "All offsets are expected to be present in "
                             "a constant expression of getelementptr.");

            const llvm::PointerType &pointerType =
                llvmCast<const llvm::PointerType>(*constantExpr->getType());

            Pointer::InclusionBased *constPointer = new Pointer::InclusionBased(
                environment.mModule, pointerType.getElementType());
            constPointer->addTarget(Pointer::Target::GlobalVariable,
                                    &instruction,
                                    *constantExpr->op_begin(),
                                    offsets,
                                    NULL);

            puts(constPointer->toString().c_str());
            value = constPointer;
        }
        else
            value = new Constant(constant);
    }

    pointer.store(*value, state);

    if (deleteValue)
        delete value;
}

void
Interpreter::getelementptr(const llvm::GetElementPtrInst &instruction,
                           Stack &stack,
                           const Environment &environment)
{
    CANAL_ASSERT(instruction.getNumOperands() > 1);
    State &state = stack.getCurrentState();

    // Find the base pointer.
    Value *base = state.findVariable(*instruction.getPointerOperand());
    if (!base)
        return;

    Pointer::InclusionBased &source =
        dynamic_cast<Pointer::InclusionBased&>(*base);

    // We get offsets. Either constants or Integer::Container.
    // Pointer points either to an array (or array offset), or to a
    // struct (or struct member).  Pointer might have multiple
    // targets.
    std::vector<Value*> offsets;
    bool allOffsetsPresent = getElementPtrOffsets(offsets,
                                                  instruction.idx_begin(),
                                                  instruction.idx_end(),
                                                  state);

    if (!allOffsetsPresent)
        return;

    const llvm::PointerType *pointerType = instruction.getType();
    Pointer::InclusionBased *result = source.getElementPtr(
        offsets, pointerType->getElementType());

    state.addFunctionVariable(instruction, result);
}

void
Interpreter::trunc(const llvm::TruncInst &instruction,
                   State &state,
                   const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::zext(const llvm::ZExtInst &instruction,
                  State &state,
                  const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::sext(const llvm::SExtInst &instruction,
                  State &state,
                  const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fptrunc(const llvm::FPTruncInst &instruction,
                     State &state,
                     const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fpext(const llvm::FPExtInst &instruction,
                   State &state,
                   const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fptoui(const llvm::FPToUIInst &instruction,
                    State &state,
                    const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fptosi(const llvm::FPToSIInst &instruction,
                    State &state,
                    const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::uitofp(const llvm::UIToFPInst &instruction,
                    State &state,
                    const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::sitofp(const llvm::SIToFPInst &instruction,
                    State &state,
                    const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::ptrtoint(const llvm::PtrToIntInst &instruction,
                      State &state,
                      const Environment &environment)
{
    Value *operand = state.findVariable(*instruction.getOperand(0));
    if (!operand)
        return;

    Pointer::InclusionBased &source =
        dynamic_cast<Pointer::InclusionBased&>(*operand);

    Pointer::InclusionBased *result = source.clone();
/*  This should really be handled in integer operations.
    Pointer::PlaceTargetMap::iterator it = result->mTargets.begin();
    for (; it != result->mTargets.end(); ++it)
    {
        // If the numeric offset exists, verify that the width
        // matches.  Otherwise, create an empty offset.
        if (it->second.mNumericOffset)
        {
            llvm::IntegerType &integerType =
                llvmCast<llvm::IntegerType>(*instruction.getDestTy());

            const Integer::Container &existingInt =
                dynamic_cast<const Integer::Container&>(*it->second.mNumericOffset);
            int existingWidth = existingInt.getBitWidth();
            CANAL_ASSERT_MSG(existingWidth == integerType.getBitWidth(),
                             "Offset integer width adjustment is not "
                             "implemented.  Perhaps now is the right "
                             "time to implement it, as you have an example.");
        }
        else
        {
            it->second.mNumericOffset =
                typeToEmptyValue(*instruction.getDestTy(), environment.mModule);
        }
    }
*/
    state.addFunctionVariable(instruction, result);
}

void
Interpreter::inttoptr(const llvm::IntToPtrInst &instruction,
                      State &state,
                      const Environment &environment)
{
    Value *operand = state.findVariable(*instruction.getOperand(0));
    if (!operand)
        return;

    Pointer::InclusionBased &source =
        dynamic_cast<Pointer::InclusionBased&>(*operand);

    const llvm::PointerType &pointerType =
        llvmCast<const llvm::PointerType>(*instruction.getDestTy());

    Pointer::InclusionBased *result =
        source.bitcast(pointerType.getElementType());

    state.addFunctionVariable(instruction, result);
}

void
Interpreter::bitcast(const llvm::BitCastInst &instruction,
                     State &state,
                     const Environment &environment)
{
    const llvm::Type *sourceType = instruction.getSrcTy();
    const llvm::Type *destinationType = instruction.getDestTy();

    CANAL_ASSERT_MSG(sourceType->isPointerTy() && destinationType->isPointerTy(),
                     "Bitcast for non-pointers is not implemented yet.");

    Value *source = state.findVariable(*instruction.getOperand(0));
    if (!source)
        return;

    Pointer::InclusionBased &sourcePointer =
        dynamic_cast<Pointer::InclusionBased&>(*source);

    Value *resultPointer = sourcePointer.bitcast(destinationType);
    state.addFunctionVariable(instruction, resultPointer);
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
Interpreter::icmp(const llvm::ICmpInst &instruction,
                  State &state,
                  const Environment &environment)
{
    cmpOperation(instruction, state, &Value::icmp);
}

void
Interpreter::fcmp(const llvm::FCmpInst &instruction,
                  State &state,
                  const Environment &environment)
{
    cmpOperation(instruction, state, &Value::fcmp);
}

void
Interpreter::phi(const llvm::PHINode &instruction,
                 State &state,
                 const Environment &environment)
{
    Value *mergedValue = NULL;
    for (int i = 0; i < instruction.getNumIncomingValues(); ++i)
    {
        Constant c;
        Value *value = variableOrConstant(*instruction.getIncomingValue(i),
                                          state, c);
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
Interpreter::select(const llvm::SelectInst &instruction,
                    State &state,
                    const Environment &environment)
{
    Value *condition = state.findVariable(*instruction.getCondition());
    if (!condition)
        return;

    Constant trueConstant, falseConstant;
    Value *trueValue = variableOrConstant(*instruction.getTrueValue(),
                                          state, trueConstant);
    Value *falseValue = variableOrConstant(*instruction.getFalseValue(),
                                           state, falseConstant);

    Value *resultValue;
    const Integer::Container &conditionInt =
        dynamic_cast<const Integer::Container&>(*condition);

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
Interpreter::call(const llvm::CallInst &instruction,
                  Stack &stack,
                  const Environment &environment)
{
    interpretCall(instruction, stack, environment.mModule);
}

void
Interpreter::va_arg(const llvm::VAArgInst &instruction,
                    State &state,
                    const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

#if LLVM_MAJOR >= 3
// Instructions available since LLVM 3.0

void
Interpreter::resume(const llvm::ResumeInst &instruction,
                    State &state,
                    const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::fence(const llvm::FenceInst &instruction,
                   State &state,
                   const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::cmpxchg(const llvm::AtomicCmpXchgInst &instruction,
                     State &state,
                     const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::atomicrmw(const llvm::AtomicRMWInst &instruction,
                       State &state,
                       const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interpreter::landingpad(const llvm::LandingPadInst &instruction,
                        State &state,
                        const Environment &environment)
{
    CANAL_NOT_IMPLEMENTED();
}

#endif

} // namespace Canal
