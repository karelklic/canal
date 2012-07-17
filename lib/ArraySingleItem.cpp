#include "ArraySingleItem.h"
#include "Utils.h"
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

bool
SingleItem::operator==(const Value &value) const
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

void
SingleItem::merge(const Value &value)
{
    const SingleItem &singleItem = dynamic_cast<const SingleItem&>(value);
    CANAL_ASSERT(mValue && singleItem.mValue); // implement if necessary
    CANAL_ASSERT(mSize && singleItem.mSize); // implement if necessary
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
SingleItem::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Array::SingleItem: {" << std::endl;
    ss << "    size:" << indentExceptFirstLine(mSize->toString(state), 9) << std::endl;
    ss << "    value: " << indentExceptFirstLine(mValue->toString(state), 11) << std::endl;
    ss << "}";
    return ss.str();
}

std::vector<Value*>
SingleItem::getItems(const Value &offset) const
{
    // TODO: check if the offset fits into mSize.
    CANAL_NOT_IMPLEMENTED();
    return std::vector<Value*>();
}

void
SingleItem::set(const Value &offset, const Value &value)
{
    // TODO: check if the offset fits into mSize.
    CANAL_NOT_IMPLEMENTED();
    mValue->merge(value);
}

} // namespace Array
} // namespace Canal
