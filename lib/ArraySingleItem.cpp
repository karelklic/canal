#include "ArraySingleItem.h"
#include "Utils.h"
#include "IntegerContainer.h"

namespace Canal {
namespace Array {

SingleItem::SingleItem(const Environment &environment,
                       Domain* size,
                       Domain* value)
    : Domain(environment), mValue(value), mSize(size)
{
}

SingleItem::SingleItem(const SingleItem &value)
    : Domain(value), mValue(value.mValue), mSize(value.mSize)
{
    if (mValue)
        mValue = mValue->clone();

    if (mSize)
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

bool
SingleItem::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const SingleItem *singleItem =
        dynCast<const SingleItem*>(&value);

    if (!singleItem)
        return false;

    if ((mSize && !singleItem->mSize) || (!mSize && singleItem->mSize))
        return false;

    if ((mValue && !singleItem->mValue) || (!mValue && singleItem->mValue))
        return false;

    if (mValue && *mValue != *singleItem->mValue)
        return false;

    if (mSize && *mSize != *singleItem->mSize)
        return false;

    return true;
}

void
SingleItem::merge(const Domain &value)
{
    const SingleItem &singleItem = dynCast<const SingleItem&>(value);
    CANAL_ASSERT_MSG(mValue && singleItem.mValue,
                     "Array value must be intialized for merging");

    CANAL_ASSERT_MSG(mSize && singleItem.mSize,
                     "Array size must be initialized for merging");

    mValue->merge(*singleItem.mValue);
    mSize->merge(*singleItem.mSize);
}

size_t
SingleItem::memoryUsage() const
{
    size_t size = sizeof(SingleItem);
    size += (mValue ? mValue->memoryUsage() : 0);
    size += (mSize ? mSize->memoryUsage() : 0);
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

void
SingleItem::add(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fadd(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::sub(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fsub(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::mul(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fmul(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::udiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::sdiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fdiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::urem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::srem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::frem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::shl(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::lshr(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::ashr(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::and_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::or_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::xor_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

static void
assertOffsetFitsToArray(uint64_t offset, const Domain &size)
{
    // Get maximum size of the array.
    const Integer::Container &integerSize =
        dynCast<const Integer::Container&>(size);

    llvm::APInt unsignedMaxSize(integerSize.getBitWidth(), 0);
    bool sizeIsKnown = integerSize.unsignedMax(unsignedMaxSize);
    // The following requirement can be changed if necessary.
    CANAL_ASSERT_MSG(sizeIsKnown, "Size must be a known value.");
    CANAL_ASSERT_MSG(offset < unsignedMaxSize.getZExtValue(),
                     "Offset out of bounds.");
}

static void
assertOffsetFitsToArray(const Domain &offset, const Domain &size)
{
    // Check if the offset might point to the array.
    const Integer::Container &integerOffset =
        dynCast<const Integer::Container&>(offset);

    llvm::APInt unsignedMinOffset(integerOffset.getBitWidth(), 0);
    bool offsetIsKnown = integerOffset.unsignedMin(unsignedMinOffset);
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
    mValue->merge(value);
}

void
SingleItem::setItem(uint64_t offset, const Domain &value)
{
    assertOffsetFitsToArray(offset, *mSize);
    mValue->merge(value);
}

void
SingleItem::setZero(const llvm::Value *place)
{
    mValue->setZero(place);
}

} // namespace Array
} // namespace Canal
