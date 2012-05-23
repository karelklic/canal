#include "ArrayExactLimitedSize.h"
#include "Utils.h"
#include <sstream>

namespace Canal {
namespace Array {

ExactLimitedSize::ExactLimitedSize()
{
}

ExactLimitedSize::ExactLimitedSize(const ExactLimitedSize &exactLimitedSize)
{
    mValues = exactLimitedSize.mValues;
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = (*it)->clone();
}

ExactLimitedSize::~ExactLimitedSize()
{
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete (*it);
}

ExactLimitedSize *
ExactLimitedSize::clone() const
{
    return new ExactLimitedSize(*this);
}

bool
ExactLimitedSize::operator==(const Value &value) const
{
    const ExactLimitedSize *array = dynamic_cast<const ExactLimitedSize*>(&value);
    if (!array)
        return false;

    if (mValues.size() != array->mValues.size())
        return false;

    std::vector<Value*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = array->mValues.begin();
    for (; itA != itAend; ++itA, ++itB)
    {
        if (*itA != *itB)
            return false;
    }

    return true;
}

void
ExactLimitedSize::merge(const Value &value)
{
    const ExactLimitedSize &array = dynamic_cast<const ExactLimitedSize&>(value);
    CANAL_ASSERT(mValues.size() == array.mValues.size());
    std::vector<Value*>::iterator itA = mValues.begin();
    std::vector<Value*>::const_iterator itAend = mValues.end(),
        itB = array.mValues.begin();
    for (; itA != itAend; ++itA, ++itB)
        (*itA)->merge(**itB);
}

size_t
ExactLimitedSize::memoryUsage() const
{
    size_t size = sizeof(ExactLimitedSize);
    size += mValues.capacity() * sizeof(Value*);
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
ExactLimitedSize::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Array::ExactLimitedSize: {" << std::endl;
    ss << "}";
    return ss.str();
}

} // namespace Array
} // namespace Canal
