#include "Constructors.h"
#include "ProductVector.h"
#include "IntegerBitfield.h"
#include "IntegerSet.h"
#include "IntegerInterval.h"
#include "IntegerUtils.h"
#include "Utils.h"
#include "ArraySingleItem.h"
#include "ArrayExactSize.h"
#include "ArrayStringPrefix.h"
#include "ArrayStringSuffix.h"
#include "ArrayStringTrie.h"
#include "FloatInterval.h"
#include "FloatUtils.h"
#include "Pointer.h"
#include "PointerUtils.h"
#include "Structure.h"
#include "Environment.h"
#include "State.h"

namespace Canal {

Constructors::Constructors(Environment &environment)
    : mEnvironment(environment)
{
    environment.setConstructors(this);
}

Domain *
Constructors::create(const llvm::Type &type) const
{
    CANAL_ASSERT_MSG(!type.isVoidTy(), "Cannot create value of type Void.");

    if (type.isIntegerTy())
    {
        llvm::IntegerType &integerType = checkedCast<llvm::IntegerType>(type);
        return createInteger(integerType.getBitWidth());
    }

    if (type.isFloatingPointTy())
    {
        const llvm::fltSemantics &semantics =
            Float::Utils::getSemantics(type);

        return createFloat(semantics);
    }

    if (type.isPointerTy())
    {
        const llvm::PointerType &pointerType =
            checkedCast<llvm::PointerType>(type);

        return createPointer(pointerType);
    }

    if (type.isArrayTy() || type.isVectorTy())
    {
        const llvm::SequentialType &stype =
            checkedCast<llvm::SequentialType>(type);

        return createArray(stype);
    }

    if (type.isStructTy())
    {
        const llvm::StructType &structType =
            checkedCast<llvm::StructType>(type);

        std::vector<Domain*> members;
        for (unsigned i = 0; i < structType.getNumElements(); i ++)
            members.push_back(create(*structType.getElementType(i)));

        return createStructure(structType, members);
    }

    CANAL_DIE_MSG("Unsupported llvm::Type::TypeID: " << type.getTypeID());
}

Domain *
Constructors::create(const llvm::Constant &value,
                     const llvm::Value &place,
                     const State *state) const
{
    if (llvm::isa<llvm::UndefValue>(value))
        return create(*value.getType());

    if (llvm::isa<llvm::ConstantInt>(value))
    {
        const llvm::ConstantInt &intValue =
            checkedCast<llvm::ConstantInt>(value);

        const llvm::APInt &i = intValue.getValue();
        return createInteger(i);
    }

    if (llvm::isa<llvm::ConstantPointerNull>(value))
    {
        const llvm::ConstantPointerNull &nullValue =
            checkedCast<llvm::ConstantPointerNull>(value);

        const llvm::PointerType &pointerType = *nullValue.getType();
        Domain *constPointer = createPointer(pointerType);
        constPointer->setZero(&place);
        return constPointer;
    }

    if (llvm::isa<llvm::ConstantExpr>(value))
    {
        const llvm::ConstantExpr &exprValue =
            checkedCast<llvm::ConstantExpr>(value);

        return createConstantExpr(exprValue, place, state);
    }

    if (llvm::isa<llvm::ConstantFP>(value))
    {
        const llvm::ConstantFP &fp = checkedCast<llvm::ConstantFP>(value);
        const llvm::APFloat &number = fp.getValueAPF();
        return createFloat(number);
    }

    if (llvm::isa<llvm::ConstantStruct>(value))
    {
        const llvm::ConstantStruct &structValue =
            checkedCast<llvm::ConstantStruct>(value);

        uint64_t elementCount = structValue.getType()->getNumElements();
        std::vector<Domain*> members;
        for (uint64_t i = 0; i < elementCount; ++i)
        {
            members.push_back(create(*structValue.getOperand(i),
                                     *structValue.getOperand(i),
                                     state));
        }

        return createStructure(*structValue.getType(), members);
    }

    if (llvm::isa<llvm::ConstantVector>(value))
    {
        const llvm::ConstantVector &vectorValue =
            checkedCast<llvm::ConstantVector>(value);

        // VectorType::getNumElements returns unsigned int.
        unsigned elementCount = vectorValue.getType()->getNumElements();
        std::vector<Domain*> values;
        for (unsigned i = 0; i < elementCount; ++i)
        {
            values.push_back(create(*vectorValue.getOperand(i),
                                    *vectorValue.getOperand(i),
                                    state));
        }

        return createArray(*vectorValue.getType(), values);
    }

    if (llvm::isa<llvm::ConstantArray>(value))
    {
        const llvm::ConstantArray &arrayValue =
            checkedCast<llvm::ConstantArray>(value);

        // ArrayType::getNumElements returns uint64_t.
        uint64_t elementCount = arrayValue.getType()->getNumElements();
        std::vector<Domain*> values;
        for (uint64_t i = 0; i < elementCount; ++i)
        {
            values.push_back(create(*arrayValue.getOperand(i),
                                    *arrayValue.getOperand(i),
                                    state));
        }

        return createArray(*arrayValue.getType(), values);
    }

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 1) || LLVM_VERSION_MAJOR > 3
    // llvm::isa<llvm::ConstantDataSequential> returns false for an
    // llvm::ConstantDataArray/Vector instance at least on on LLVM
    // 3.1.
    if (llvm::isa<llvm::ConstantDataVector>(value) ||
        llvm::isa<llvm::ConstantDataArray>(value))
    {
         const llvm::ConstantDataSequential &sequentialValue =
            checkedCast<llvm::ConstantDataSequential>(value);

        unsigned elementCount = sequentialValue.getNumElements();
        std::vector<Domain*> values;
        for (unsigned i = 0; i < elementCount; ++i)
        {
            values.push_back(create(*sequentialValue.getElementAsConstant(i),
                                    place,
                                    state));
        }

        return createArray(*sequentialValue.getType(), values);
    }
#endif

    if (llvm::isa<llvm::ConstantAggregateZero>(value))
    {
        const llvm::Type *type = value.getType();
        Domain *result = Constructors::create(*type);
        result->setZero(&place);
        return result;
    }

    if (llvm::isa<llvm::Function>(value))
    {
        const llvm::Function &functionValue =
            checkedCast<llvm::Function>(value);

        Domain *constPointer;
        constPointer = createPointer(*llvm::PointerType::getUnqual(
                                         functionValue.getFunctionType()));

        Pointer::Utils::addTarget(*constPointer,
                                  Pointer::Target::Function,
                                  &place,
                                  &value,
                                  std::vector<Domain*>(),
                                  NULL);

        return constPointer;
    }

    CANAL_NOT_IMPLEMENTED();
}

Domain *
Constructors::createInteger(unsigned bitWidth) const
{
    Product::Vector* container = new Product::Vector(mEnvironment);
    container->mValues.push_back(new Integer::Bitfield(mEnvironment, bitWidth));
    container->mValues.push_back(new Integer::Set(mEnvironment, bitWidth));
    container->mValues.push_back(new Integer::Interval(mEnvironment, bitWidth));
    return container;
}

Domain *
Constructors::createInteger(const llvm::APInt &number) const
{
    Product::Vector* container = new Product::Vector(mEnvironment);
    container->mValues.push_back(new Integer::Bitfield(mEnvironment, number));
    container->mValues.push_back(new Integer::Set(mEnvironment, number));
    container->mValues.push_back(new Integer::Interval(mEnvironment, number));
    return container;
}

Domain *
Constructors::createFloat(const llvm::fltSemantics &semantics) const
{
    return new Float::Interval(mEnvironment, semantics);
}

Domain *
Constructors::createFloat(const llvm::APFloat &number) const
{
    return new Float::Interval(mEnvironment, number);
}

Domain *
Constructors::createArray(const llvm::SequentialType &type) const
{
    Product::Vector *container = new Product::Vector(mEnvironment);
    container->mValues.push_back(new Array::ExactSize(mEnvironment, type));
    container->mValues.push_back(new Array::SingleItem(mEnvironment, type));
    container->mValues.push_back(new Array::StringPrefix(mEnvironment, type));
    container->mValues.push_back(new Array::StringSuffix(mEnvironment, type));
    //container->mValues.push_back(new Array::StringTrie(mEnvironment, type));
    return container;
}

Domain *
Constructors::createArray(const llvm::SequentialType &type,
                          Domain *size) const
{
    Product::Vector *container = new Product::Vector(mEnvironment);
    container->mValues.push_back(new Array::ExactSize(mEnvironment, type));
    container->mValues.push_back(new Array::SingleItem(mEnvironment, type, size));
    container->mValues.push_back(new Array::StringPrefix(mEnvironment, type));
    //container->mValues.push_back(new Array::StringSuffix(mEnvironment, type));
    //container->mValues.push_back(new Array::StringTrie(mEnvironment, type));
    return container;
}

Domain *
Constructors::createArray(const llvm::SequentialType &type,
                          const std::vector<Domain*> &values) const
{
    Product::Vector *container = new Product::Vector(mEnvironment);
    container->mValues.push_back(new Array::ExactSize(mEnvironment, type, values));
    container->mValues.push_back(new Array::SingleItem(mEnvironment, type, values.begin(), values.end()));
    container->mValues.push_back(new Array::StringPrefix(mEnvironment, type, values.begin(), values.end()));
    //container->mValues.push_back(new Array::StringSuffix(mEnvironment, type, values.begin(), values.end()));
    //container->mValues.push_back(new Array::StringTrie(mEnvironment, type, values.begin(), values.end()));
    return container;
}

Domain *
Constructors::createPointer(const llvm::PointerType &type) const
{
    return new Pointer::Pointer(mEnvironment, type);
}

Domain *
Constructors::createStructure(const llvm::StructType &type) const
{
    return new Structure(mEnvironment, type);
}

Domain *
Constructors::createStructure(const llvm::StructType &type,
                              const std::vector<Domain*> &members) const
{
    return new Structure(mEnvironment, type, members);
}

Domain *
Constructors::createConstantExpr(const llvm::ConstantExpr &value,
                                 const llvm::Value &place,
                                 const State *state) const
{
    CANAL_ASSERT_MSG(state,
                     "State is mandatory for constant expressions.");

    std::vector<const Domain*> operands;
    std::vector<bool> operandsDelete;
    llvm::ConstantExpr::const_op_iterator it = value.op_begin(),
        itend = value.op_end();

    for (; it != itend; ++it)
    {
        const Domain *variable;
        if (llvm::isa<llvm::GlobalValue>(**it))
        {
            variable = state->findVariable(**it);
            operandsDelete.push_back(false);
        }
        else
        {
            variable = create(checkedCast<llvm::Constant>(**it),
                              place,
                              state);

            operandsDelete.push_back(true);
        }

        CANAL_ASSERT_MSG(variable, "It is expected that variable used"
                         " in constant expressions is available.\n"
                         "Missing: \"" << *it << "\"\n"
                         << "In \"" << value << "\"\n"
                         << "On line \"" << place << "\"");

        operands.push_back(variable);
    }

    Domain *result = NULL;
    switch (value.getOpcode())
    {
    case llvm::Instruction::GetElementPtr:
        result = createGetElementPtr(value, operands, place);
        break;
    case llvm::Instruction::BitCast:
        result = createBitCast(value, operands, place);
        break;
    default:
        CANAL_FATAL_ERROR("Constant Expressions Instruction not implemented: "
                          << value.getOpcodeName());
    }

    std::vector<const Domain*>::const_iterator dit = operands.begin(),
        ditend = operands.end();

    std::vector<bool>::const_iterator ddit = operandsDelete.begin();
    for (; dit != ditend; ++dit, ++ddit)
    {
        if (*ddit)
            delete *dit;
    }

    return result;
}

Domain *
Constructors::createGetElementPtr(const llvm::ConstantExpr &value,
                                  const std::vector<const Domain*> &operands,
                                  const llvm::Value &place) const
{
    std::vector<Domain*> offsets;
    std::vector<const Domain*>::const_iterator it = operands.begin() + 1,
        itend = operands.end();

    for (; it != itend; ++it)
    {
        unsigned bitWidth = Integer::Utils::getBitWidth(**it);
        CANAL_ASSERT_MSG(bitWidth <= 64,
                         "Cannot handle GetElementPtr offset"
                         " with more than 64 bits.");

        if (bitWidth < 64)
        {
            Domain *offset = createInteger(64);
            offset->zext(**it);
            offsets.push_back(offset);
        }
        else
            offsets.push_back((*it)->clone());
    }

    const llvm::PointerType &pointerType =
        checkedCast<const llvm::PointerType>(*value.getType());

    // GetElementPtr on a Pointer
    const Pointer::Pointer *pointer = dynCast<Pointer::Pointer>(*operands.begin());
    if (pointer)
    {
        return pointer->getElementPtr(offsets,
                                      pointerType,
                                      *this);
    }
    else
    {
        // GetElementPtr on anything except a pointer.  For example, it is
        // called on arrays and structures.
        Domain *result = createPointer(pointerType);
        Pointer::Utils::addTarget(*result,
                                  Pointer::Target::Block,
                                  &place,
                                  *value.op_begin(),
                                  offsets,
                                  NULL);

        return result;
    }
}

Domain *
Constructors::createBitCast(const llvm::ConstantExpr &value,
                            const std::vector<const Domain*> &operands,
                            const llvm::Value &place) const
{
    // BitCast from Pointer.  It is always a bitcast to some other
    // pointer.
    const Pointer::Pointer *pointer = dynCast<Pointer::Pointer>(*operands.begin());
    const llvm::PointerType *pointerType =
        checkedCast<llvm::PointerType>(value.getType());

    if (pointer)
    {
        CANAL_ASSERT(pointerType);
        return pointer->bitcast(*pointerType);
    }

    // BitCast from anything to a pointer.
    if (pointerType)
    {
        Domain *result;
        result = createPointer(*pointerType);

        Pointer::Utils::addTarget(*result,
                                  Pointer::Target::Block,
                                  &place,
                                  *value.op_begin(),
                                  std::vector<Domain*>(),
                                  NULL);

        return result;
    }

    // BitCast from non-pointer to another non-pointer.
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
