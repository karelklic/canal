#include "ArraySingleItem.h"
#include "Utils.h"
#include "IntegerContainer.h"
#include "Environment.h"
#include "Constructors.h"
#include "IntegerUtils.h"

namespace Canal {
namespace Array {

SingleItem::SingleItem(const Environment &environment,
                       const llvm::SequentialType &type)
    : Domain(environment), mType(type)
{
    const llvm::Type &elementType = *type.getElementType();
    mValue = environment.getConstructors().create(elementType);

    uint64_t count = 0;

    const llvm::ArrayType *array = llvm::dyn_cast<llvm::ArrayType>(&type);
    if (array)
        count = array->getNumElements();

    const llvm::VectorType *vector = llvm::dyn_cast<llvm::VectorType>(&type);
    if (vector)
        count = vector->getNumElements();

    mSize = environment.getConstructors().createInteger(llvm::APInt(64, count));
    if (count == 0)
        mSize->setTop();
}

SingleItem::SingleItem(const Environment &environment,
                       const llvm::SequentialType &type,
                       const std::vector<Domain*> &values)
    : Domain(environment), mType(type)
{
    mSize = environment.getConstructors().createInteger(llvm::APInt(64, values.size()));

    const llvm::Type &elementType = *type.getElementType();
    mValue = environment.getConstructors().create(elementType);

    std::vector<Domain*>::const_iterator it = values.begin(),
        itend = values.end();

    for (; it != itend; ++it)
    {
        mValue->join(**it);
        delete *it;
    }
}

SingleItem::SingleItem(const Environment &environment,
                       const llvm::SequentialType &type,
                       Domain* size)
    : Domain(environment), mSize(size), mType(type)
{
    const llvm::Type &elementType = *type.getElementType();
    mValue = environment.getConstructors().create(elementType);
}

SingleItem::SingleItem(const SingleItem &value)
    : Domain(value), mValue(value.mValue), mSize(value.mSize), mType(value.mType)
{
    mValue = mValue->clone();
    mSize = mSize->clone();
}

SingleItem::~SingleItem()
{
    delete mValue;
    delete mSize;
}

SingleItem *
SingleItem::clone() const
{
    return new SingleItem(*this);
}

size_t
SingleItem::memoryUsage() const
{
    size_t size = sizeof(SingleItem);
    size += mValue->memoryUsage();
    size += mSize->memoryUsage();
    return size;
}

std::string
SingleItem::toString() const
{
    StringStream ss;
    ss << "arraySingleItem\n";
    ss << "    size\n";
    ss << indent(mSize->toString(), 8);
    ss << "    value\n";
    ss << indent(mValue->toString(), 8);
    return ss.str();
}

bool
SingleItem::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const SingleItem *singleItem =
        dynCast<const SingleItem*>(&value);

    if (!singleItem)
        return false;

    if (*mValue != *singleItem->mValue)
        return false;

    if (*mSize != *singleItem->mSize)
        return false;

    return true;
}

bool
SingleItem::operator<(const Domain& value) const
{
    if (this == &value)
        return false;

    const SingleItem *singleItem =
        dynCast<const SingleItem*>(&value);

    if (!singleItem)
        return false;

    if (!(*mValue < *singleItem->mValue))
        return false;

    if (!(*mSize < *singleItem->mSize))
        return false;

    return true;
}

bool
SingleItem::operator>(const Domain& value) const
{
    if (this == &value)
        return false;

    const SingleItem *singleItem =
        dynCast<const SingleItem*>(&value);

    if (!singleItem)
        return false;

    if (!(*mValue > *singleItem->mValue))
        return false;

    if (!(*mSize > *singleItem->mSize))
        return false;

    return true;
}

SingleItem &
SingleItem::join(const Domain &value)
{
    const SingleItem &array = dynCast<const SingleItem&>(value);
    mValue->join(*array.mValue);
    mSize->join(*array.mSize);
    return *this;
}

SingleItem &
SingleItem::meet(const Domain &value)
{
    const SingleItem &array = dynCast<const SingleItem&>(value);
    mValue->meet(*array.mValue);
    mSize->meet(*array.mSize);
    return *this;
}

bool
SingleItem::isBottom() const
{
    return mValue->isBottom();
}

void
SingleItem::setBottom()
{
    mValue->setBottom();
}

bool
SingleItem::isTop() const
{
    return mValue->isTop();
}

void
SingleItem::setTop()
{
    mValue->setTop();
}

float
SingleItem::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

static SingleItem &
binaryOperation(SingleItem &result,
                const Domain &a,
                const Domain &b,
                Domain::BinaryOperation operation)
{
    const SingleItem &aa = dynCast<const SingleItem&>(a),
        &bb = dynCast<const SingleItem&>(b);

    ((result.mValue)->*(operation))(*aa.mValue, *bb.mValue);
    return result;
}

SingleItem &
SingleItem::add(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::add);
}

SingleItem &
SingleItem::fadd(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fadd);
}

SingleItem &
SingleItem::sub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sub);
}

SingleItem &
SingleItem::fsub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fsub);
}

SingleItem &
SingleItem::mul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::mul);
}

SingleItem &
SingleItem::fmul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fmul);
}

SingleItem &
SingleItem::udiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::udiv);
}

SingleItem &
SingleItem::sdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sdiv);
}

SingleItem &
SingleItem::fdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fdiv);
}

SingleItem &
SingleItem::urem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::urem);
}

SingleItem &
SingleItem::srem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::srem);
}

SingleItem &
SingleItem::frem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::frem);
}

SingleItem &
SingleItem::shl(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::shl);
}

SingleItem &
SingleItem::lshr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::lshr);
}

SingleItem &
SingleItem::ashr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::ashr);
}

SingleItem &
SingleItem::and_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::and_);
}

SingleItem &
SingleItem::or_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::or_);
}

SingleItem &
SingleItem::xor_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::xor_);
}

SingleItem &
SingleItem::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

SingleItem &
SingleItem::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

const llvm::SequentialType &
SingleItem::getValueType() const
{
    return mType;
}

Domain *
SingleItem::getValueAbstractSize() const
{
    return mSize->clone();
}

Domain *
SingleItem::getValueCell(uint64_t offset) const
{
    Domain *cell = mEnvironment.getConstructors().createInteger(8);
    cell->setTop();
    return cell;
}

void
SingleItem::mergeValueCell(uint64_t offset, const Domain &value)
{
    setTop();
}

static void
assertOffsetFitsToArray(uint64_t offset, const Domain &size)
{
    llvm::APInt unsignedMaxSize(Integer::Utils::getBitWidth(size), 0);
    bool sizeIsKnown = Integer::Utils::unsignedMax(size, unsignedMaxSize);
    // The following requirement can be changed if necessary.
    CANAL_ASSERT_MSG(sizeIsKnown, "Size must be a known value.");
    CANAL_ASSERT_MSG(offset < unsignedMaxSize.getZExtValue(),
                     "Offset out of bounds.");
}

static void
assertOffsetFitsToArray(const Domain &offset, const Domain &size)
{
    llvm::APInt unsignedMinOffset(Integer::Utils::getBitWidth(offset), 0);
    bool offsetIsKnown = Integer::Utils::unsignedMin(offset, unsignedMinOffset);
    // The following requirement can be changed if necessary.
    CANAL_ASSERT_MSG(offsetIsKnown, "Offset must be a known value.");
    assertOffsetFitsToArray(unsignedMinOffset.getZExtValue(), size);
}

std::vector<Domain*>
SingleItem::getItem(const Domain &offset) const
{
    assertOffsetFitsToArray(offset, *mSize);
    std::vector<Domain*> result;
    result.push_back(mValue);
    return result;
}

Domain *
SingleItem::getItem(uint64_t offset) const
{
    assertOffsetFitsToArray(offset, *mSize);
    return mValue;
}

void
SingleItem::setItem(const Domain &offset, const Domain &value)
{
    assertOffsetFitsToArray(offset, *mSize);
    mValue->join(value);
}

void
SingleItem::setItem(uint64_t offset, const Domain &value)
{
    assertOffsetFitsToArray(offset, *mSize);
    mValue->join(value);
}

void
SingleItem::setZero(const llvm::Value *place)
{
    mValue->setZero(place);
}

} // namespace Array
} // namespace Canal
