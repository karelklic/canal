#include "Constructors.h"
#include "IntegerContainer.h"
#include "Utils.h"
#include "ArraySingleItem.h"
#include "ArrayExactSize.h"
#include "FloatInterval.h"
#include "Pointer.h"
#include "Structure.h"
#include "State.h"
#include <llvm/Constants.h>
#include <stdio.h>

namespace Canal {

Constructors::Constructors(const Environment &environment)
    : mEnvironment(environment)
{
}

Domain *
Constructors::create(const llvm::Type &type) const
{
    CANAL_ASSERT_MSG(!type.isVoidTy(), "Cannot create value of type Void.");

    if (type.isIntegerTy())
    {
        llvm::IntegerType &integerType = llvmCast<llvm::IntegerType>(type);
        return new Integer::Container(mEnvironment, integerType.getBitWidth());
    }

    if (type.isFloatingPointTy())
    {
        const llvm::fltSemantics *semantics = getFloatingPointSemantics(type);
        return new Float::Interval(mEnvironment, *semantics);
    }

    if (type.isPointerTy())
    {
        const llvm::PointerType &pointerType =
            llvmCast<const llvm::PointerType>(type);

        CANAL_ASSERT_MSG(pointerType.getElementType(),
                         "Element type must be known.");

        return new Pointer::InclusionBased(mEnvironment,
                                           *pointerType.getElementType());
    }

    if (type.isArrayTy() || type.isVectorTy())
    {
        uint64_t size;
        if (type.isArrayTy()) {
            const llvm::ArrayType &arrayType = llvm::cast<llvm::ArrayType>(type);
            size = arrayType.getNumElements();
        }
        else {
            const llvm::VectorType &vectorType = llvm::cast<llvm::VectorType>(type);
            size = vectorType.getNumElements();
        }
        Domain* value = this->create(*type.getContainedType(0));
        return new Array::ExactSize(mEnvironment, size, value);
    }

    if (type.isStructTy())
    {
        const llvm::StructType &structType =
            llvmCast<llvm::StructType>(type);

        std::vector<Domain*> members;
        for (unsigned i = 0; i < structType.getNumElements(); i ++)
            members.push_back(create(*structType.getElementType(i)));

        Structure *structure = new Structure(mEnvironment, members);
        return structure;
    }

    CANAL_DIE_MSG("Unsupported llvm::Type::TypeID: " << type.getTypeID());
}

Domain *
Constructors::create(const llvm::Constant &value,
                     const State *state) const
{
    if (llvm::isa<llvm::ConstantInt>(value))
    {
        const llvm::ConstantInt &intValue =
            llvmCast<llvm::ConstantInt>(value);

        const llvm::APInt &i = intValue.getValue();
        return new Integer::Container(mEnvironment, i);
    }

    if (llvm::isa<llvm::ConstantPointerNull>(value))
    {
        const llvm::ConstantPointerNull &nullValue =
            llvmCast<llvm::ConstantPointerNull>(value);

        const llvm::PointerType &pointerType = *nullValue.getType();
        Pointer::InclusionBased *constPointer = new Pointer::InclusionBased(
            mEnvironment, *pointerType.getElementType());

        constPointer->addTarget(Pointer::Target::Constant,
                                &value,
                                NULL,
                                std::vector<Domain*>(),
                                NULL);

        return constPointer;
    }

    if (llvm::isa<llvm::ConstantExpr>(value))
    {
        const llvm::ConstantExpr &exprValue = llvmCast<llvm::ConstantExpr>(value);
        CANAL_ASSERT_MSG(state, "State is mandatory for constant expressions.");
        Domain *variable = state->findVariable(**value.op_begin());
        CANAL_ASSERT_MSG(variable, "It is expected that variable used"
                         " in constant expressions is available.");

        switch (exprValue.getOpcode())
        {
        case llvm::Instruction::GetElementPtr:
            return createGetElementPtr(exprValue, *variable);
        case llvm::Instruction::BitCast:
            return createBitCast(exprValue, *variable);
        default:
            CANAL_NOT_IMPLEMENTED();
        }
    }

    if (llvm::isa<llvm::ConstantFP>(value))
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantStruct>(value))
    {
        const llvm::ConstantStruct &structValue = llvmCast<llvm::ConstantStruct>(value);
        uint64_t elementCount = structValue.getType()->getNumElements();
        std::vector<Domain*> members;
        for (uint64_t i = 0; i < elementCount; ++i)
            members.push_back(create(*structValue.getOperand(i), state));

        return new Structure(mEnvironment, members);
    }

    if (llvm::isa<llvm::ConstantVector>(value))
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantArray>(value))
    {
        const llvm::ConstantArray &arrayValue = llvmCast<llvm::ConstantArray>(value);
        uint64_t elementCount = arrayValue.getType()->getNumElements();
        std::vector<Domain*> values;
        for (uint64_t i = 0; i < elementCount; ++i)
            values.push_back(create(*arrayValue.getOperand(i), state));

        return new Array::ExactSize(mEnvironment, values);
    }

    if (llvm::isa<llvm::ConstantAggregateZero>(value))
    {
        const llvm::Type *type = value.getType();
        Domain *result = Constructors::create(*type);
        result->setZero(&value);
        return result;
    }

    CANAL_DIE_MSG("not implemented for " << typeid(value).name());
}

const llvm::fltSemantics *
Constructors::getFloatingPointSemantics(const llvm::Type &type)
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

Domain *
Constructors::createGetElementPtr(const llvm::ConstantExpr &value,
                                  const Domain &variable) const
{
    std::vector<Domain*> offsets;
    llvm::ConstantExpr::const_op_iterator it = value.op_begin() + 1;
    for (; it != value.op_end(); ++it)
    {
        const llvm::ConstantInt &constant =
            llvmCast<llvm::ConstantInt>(**it);

        offsets.push_back(create(constant, NULL));
    }

    const llvm::PointerType &pointerType =
        llvmCast<const llvm::PointerType>(*value.getType());

    // GetElementPtr on a Pointer
    const Pointer::InclusionBased *pointer =
        dynCast<const Pointer::InclusionBased*>(&variable);

    if (pointer)
        return pointer->getElementPtr(offsets, *pointerType.getElementType());

    // GetElementPtr on anything except a pointer.  For example, it is
    // used for arrays.
    Pointer::InclusionBased *result =
        new Pointer::InclusionBased(mEnvironment, *pointerType.getElementType());

    result->addTarget(Pointer::Target::GlobalVariable,
                      &value,
                      *value.op_begin(),
                      offsets,
                      NULL);

    return result;
}

Domain *
Constructors::createBitCast(const llvm::ConstantExpr &value,
                            const Domain &variable) const
{
    // BitCast from Pointer.  It is always a bitcast to some other
    // pointer.
    const Pointer::InclusionBased *pointer =
        dynCast<const Pointer::InclusionBased*>(&variable);

    const llvm::PointerType *pointerType =
        llvmCast<const llvm::PointerType>(value.getType());

    if (pointer)
    {
        CANAL_ASSERT(pointerType);
        return pointer->bitcast(*pointerType->getElementType());
    }

    // BitCast from anything to a pointer.
    if (pointerType)
    {
        Pointer::InclusionBased *result =
            new Pointer::InclusionBased(mEnvironment,
                                        *pointerType->getElementType());

        result->addTarget(Pointer::Target::GlobalVariable,
                          &value,
                          *value.op_begin(),
                          std::vector<Domain*>(),
                          NULL);

        return result;
    }

    // BitCast from non-pointer to another non-pointer.
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
