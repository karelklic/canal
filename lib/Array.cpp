#include "Array.h"

namespace Canal {
namespace Array {

SingleItem::SingleItem() : mItemValue(NULL), mSize(NULL)
{
}

SingleItem::~SingleItem()
{
    delete mItemValue;
    delete mSize;
}

Value *SingleItem::clone() const
{
    SingleItem *copy = new SingleItem();
    copy->mItemValue = mItemValue->clone();
    copy->mSize = mSize->clone();
    return copy;
}

bool SingleItem::operator==(const Value &value) const
{
    const SingleItem *singleItem = dynamic_cast<const SingleItem*>(&value);
    if (!singleItem)
        return false;

    return *mItemValue == *singleItem->mItemValue
        && *mSize == *singleItem->mSize;
}

void SingleItem::merge(const Value &value)
{
    const SingleItem &singleItem = dynamic_cast<const SingleItem&>(value);
    mItemValue->merge(*singleItem.mItemValue);
    mSize->merge(*singleItem.mSize);
}

size_t SingleItem::memoryUsage() const
{
    return sizeof(SingleItem) + mItemValue->memoryUsage() + mSize->memoryUsage();
}

void SingleItem::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Array::SingleItem(size:" << mSize << ")";
}

} // namespace Array
} // namespace Canal
