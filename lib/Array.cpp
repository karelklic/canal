#include "Array.h"
#include "Utils.h"

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
    if ((mValue && !singleItem->mValue) || (!mValue && singleItem->mValue))
        return false;

    if (mValue && *mValue != *singleItem->mValue)
        return false;
    if (mSize && *mSize != *singleItem->mSize)
        return false;

    return true;
}

void SingleItem::merge(const Value &value)
{
    const SingleItem &singleItem = dynamic_cast<const SingleItem&>(value);
    CANAL_ASSERT(mValue && singleItem.mValue); // implement if necessary
    CANAL_ASSERT(mSize && singleItem.mSize); // implement if necessary
    mValue->merge(*singleItem.mValue);
    mSize->merge(*singleItem.mSize);
}

size_t SingleItem::memoryUsage() const
{
    size_t size = sizeof(SingleItem);
    size += (mValue ? mValue->memoryUsage() : 0);
    size += (mSize ? mSize->memoryUsage() : 0);
    return size;
}

void SingleItem::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Array::SingleItem(size:" << mSize << ")";
}

} // namespace Array
} // namespace Canal
