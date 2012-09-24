#include "Constructors.h"
#include "IntegerContainer.h"
#include "Utils.h"
#include "ArraySingleItem.h"
#include "ArrayExactSize.h"
#include "FloatInterval.h"
#include "Pointer.h"
#include "Structure.h"
#include <llvm/Constants.h>

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
        return new Array::SingleItem(mEnvironment);
    }

    if (type.isStructTy())
    {
        const llvm::StructType &structType =
            llvmCast<llvm::StructType>(type);

        Structure *structure = new Structure(mEnvironment);

        llvm::StructType::element_iterator it = structType.element_begin(),
            itend = structType.element_end();

        for (; it != itend; ++it)
            structure->mMembers.push_back(create(**it));

        return structure;
    }

    CANAL_DIE_MSG("Unsupported llvm::Type::TypeID: " << type.getTypeID());
}

Domain *
Constructors::create(const llvm::Constant &value) const
{
    if (llvm::isa<llvm::ConstantInt>(value))
    {
        const llvm::ConstantInt &intValue = llvmCast<llvm::ConstantInt>(value);
        const llvm::APInt &i = intValue.getValue();

        return new Integer::Container(mEnvironment, i);
    }

    if (llvm::isa<llvm::ConstantPointerNull>(value))
    {
        const llvm::ConstantPointerNull &nullValue =
            llvmCast<llvm::ConstantPointerNull>(value);

        const llvm::Type &type = *nullValue.getType();

        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantExpr>(value))
    {
        const llvm::ConstantExpr &exprValue = llvmCast<llvm::ConstantExpr>(value);
        if (exprValue.getOpcode() == llvm::Instruction::GetElementPtr)
        {
            CANAL_NOT_IMPLEMENTED();
        }

        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantFP>(value))
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantStruct>(value))
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantVector>(value))
    {
        CANAL_NOT_IMPLEMENTED();
    }

    if (llvm::isa<llvm::ConstantArray>(value))
    {
        const llvm::ConstantArray &arrayValue = llvmCast<llvm::ConstantArray>(value);
        uint64_t elementCount = arrayValue.getType()->getNumElements();

        Array::ExactSize *result = new Array::ExactSize(mEnvironment);

        for (unsigned i = 0; i < elementCount; ++i)
            result->mValues.push_back(create(*arrayValue.getOperand(i)));

        return result;
    }

    if (llvm::isa<llvm::ConstantAggregateZero>(value))
    {
        CANAL_NOT_IMPLEMENTED();
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

} // namespace Canal
