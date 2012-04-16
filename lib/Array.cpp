#include "Array.h"
#include "Utils.h"

namespace Canal {
namespace Array {

SingleItem::SingleItem() : mItemValue(NULL), mSize(NULL)
{
}

SingleItem::SingleItem(const SingleItem &singleItem)
{
    mItemValue = singleItem.mItemValue;
    if (mItemValue)
        mItemValue = mItemValue->clone();

    mSize = singleItem.mSize;
    if (mSize)
        mSize = mSize->clone();
}

SingleItem::~SingleItem()
{
    delete mItemValue;
    delete mSize;
}

SingleItem *SingleItem::clone() const
{
    return new SingleItem(*this);
}

bool SingleItem::operator==(const Value &value) const
{
    const SingleItem *singleItem = dynamic_cast<const SingleItem*>(&value);
    if (!singleItem)
        return false;

    if ((mSize && !singleItem->mSize) || (!mSize && singleItem->mSize))
        return false;
    if ((mItemValue && !singleItem->mItemValue) || (!mItemValue && singleItem->mItemValue))
        return false;

    if (mItemValue && *mItemValue != *singleItem->mItemValue)
        return false;
    if (mSize && *mSize != *singleItem->mSize)
        return false;

    return true;
}

void SingleItem::merge(const Value &value)
{
    const SingleItem &singleItem = dynamic_cast<const SingleItem&>(value);
    CANAL_ASSERT(mItemValue && singleItem.mItemValue); // implement if necessary
    CANAL_ASSERT(mSize && singleItem.mSize); // implement if necessary
    mItemValue->merge(*singleItem.mItemValue);
    mSize->merge(*singleItem.mSize);
}

size_t SingleItem::memoryUsage() const
{
    size_t size = sizeof(SingleItem);
    size += (mItemValue ? mItemValue->memoryUsage() : 0);
    size += (mSize ? mSize->memoryUsage() : 0);
    return size;
}

void SingleItem::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Array::SingleItem(size:" << mSize << ")";
}

} // namespace Array
} // namespace Canal
