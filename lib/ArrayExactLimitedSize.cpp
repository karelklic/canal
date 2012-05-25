#include "ArrayExactLimitedSize.h"
#include "Constant.h"
#include "Integer.h"
#include "IntegerEnumeration.h"
#include "IntegerRange.h"
#include "Utils.h"
#include <sstream>
#include <llvm/Constants.h>

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

Value *
ExactLimitedSize::get(const Value *offset) const
{
    const Constant *constant = dynamic_cast<const Constant*>(offset);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        CANAL_ASSERT(constant->getAPInt().getBitWidth() <= 64);
        uint64_t numOffset = constant->getAPInt().getZExtValue();
        CANAL_ASSERT(numOffset < mValues.size());
        return mValues[numOffset];
    }

    const Integer::Container *integer = dynamic_cast<const Integer::Container*>(offset);
    // Another types of offset not supported so far.
    CANAL_ASSERT(integer);

    // First try an enumeration, then range.
    const Integer::Enumeration &enumeration = integer->getEnumeration();
    if (!enumeration.isTop())
    {
        Value *result = NULL;
        Integer::APIntSet::const_iterator it = enumeration.mValues.begin(),
            itend = enumeration.mValues.end();
        for (; it != itend; ++it)
        {
            CANAL_ASSERT(it->getBitWidth() <= 64);
            uint64_t numOffset = it->getZExtValue();

            // If some offset from the enumeration points out of the
            // array bounds, we ignore it.  It might be caused either
            // by a bug in the code, or by imprecision of the
            // interpreter.
            if (numOffset >= mValues.size())
                continue;

            if (!result)
                result = mValues[numOffset]->clone();
            else
                result->merge(*mValues[numOffset]);
        }

        // At least one of the offsets in the enumeration should point
        // to the array.  Otherwise it might be a bug in the
        // interpreter that requires investigation.
        CANAL_ASSERT(result);
        return result;
    }

    const Integer::Range &range = integer->getRange();
    // Let's care about the unsigned range only.
    if (!range.mUnsignedTop)
    {
        CANAL_ASSERT(range.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = range.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = range.mUnsignedTo.getZExtValue();
        // At least part of the range should point to the array.
        // Otherwise it might be a bug in the interpreter that
        // requires investigation.
        CANAL_ASSERT(from < mValues.size());
        Value *result = NULL;
        for (size_t loop = from; loop < mValues.size() && loop <= to; ++loop)
        {
            if (!result)
                result = mValues[loop]->clone();
            else
                result->merge(*mValues[loop]);
        }

        return result;
    }

    // Both enumeration and range are set to the top value, so merge
    // all members and return such a combined value.
    Value *result = NULL;
    std::vector<Value*>::const_iterator it = mValues.begin(),
        itend = mValues.end();
    for (; it != itend; ++it)
    {
        if (!result)
            result = (*it)->clone();
        else
            result->merge(**it);
    }
    CANAL_ASSERT(result); // Zero length arrays are not supported.
    return result;
}

void
ExactLimitedSize::set(const Value *offset, const Value *value)
{
    CANAL_NOT_IMPLEMENTED();
}


} // namespace Array
} // namespace Canal