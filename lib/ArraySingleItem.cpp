#include "ArraySingleItem.h"
#include "Utils.h"
#include "IntegerContainer.h"
#include "Constant.h"
#include <llvm/ADT/APInt.h>
#include <sstream>
#include <iostream>

namespace Canal {
namespace Array {

SingleItem::SingleItem() : mValue(NULL), mSize(NULL)
{
}

SingleItem::SingleItem(const SingleItem &singleItem)
{
    mValue = singleItem.mValue;
    if (mValue)
        mValue = mValue->clone();

    mSize = singleItem.mSize;
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

SingleItem *
SingleItem::cloneCleaned() const
{
    return new SingleItem();
}

bool
SingleItem::operator==(const Value &value) const
{
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
SingleItem::merge(const Value &value)
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
    std::stringstream ss;
    ss << "arraySingleItem" << std::endl;
    ss << "    size" << std::endl;
    ss << indent(mSize->toString(), 8);
    ss << "    value" << std::endl;
    ss << indent(mValue->toString(), 8);
    return ss.str();
}

void
SingleItem::add(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fadd(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::sub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fsub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::mul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fmul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::udiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::sdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::urem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::srem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::frem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::shl(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::lshr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::ashr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::and_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::or_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::xor_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::icmp(const Value &a, const Value &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
SingleItem::fcmp(const Value &a, const Value &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

static void
assertOffsetFitsToArray(uint64_t offset, const Value &size)
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
assertOffsetFitsToArray(const Value &offset, const Value &size)
{
    // Check if the offset might point to the array.
    if (const Constant *constant = dynCast<const Constant*>(&offset))
    {
        CANAL_ASSERT(constant->isAPInt());
        assertOffsetFitsToArray(constant->getAPInt().getZExtValue(), size);
        return;
    }
    else
    {
        const Integer::Container &integerOffset =
            dynCast<const Integer::Container&>(offset);
        llvm::APInt unsignedMinOffset(integerOffset.getBitWidth(), 0);
        bool offsetIsKnown = integerOffset.unsignedMin(unsignedMinOffset);
        // The following requirement can be changed if necessary.
        CANAL_ASSERT_MSG(offsetIsKnown, "Offset must be a known value.");
        assertOffsetFitsToArray(unsignedMinOffset.getZExtValue(), size);
    }
}

std::vector<Value*>
SingleItem::getItem(const Value &offset) const
{
    assertOffsetFitsToArray(offset, *mSize);
    std::vector<Value*> result;
    result.push_back(mValue);
    return result;
}

Value *
SingleItem::getItem(uint64_t offset) const
{
    assertOffsetFitsToArray(offset, *mSize);
    return mValue;
}

void
SingleItem::setItem(const Value &offset, const Value &value)
{
    assertOffsetFitsToArray(offset, *mSize);
    mValue->merge(value);
}

void
SingleItem::setItem(uint64_t offset, const Value &value)
{
    assertOffsetFitsToArray(offset, *mSize);
    mValue->merge(value);
}

} // namespace Array
} // namespace Canal
