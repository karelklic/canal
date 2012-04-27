#include "IntegerRange.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Range::Range(unsigned numBits) : mEmpty(true), mFrom(numBits, 0), mFromInfinity(false), mTo(numBits, 0), mToInfinity(false)
{
}

Range *
Range::clone() const
{
    return new Range(*this);
}

bool
Range::operator==(const Value& value) const
{
    const Range *range = dynamic_cast<const Range*>(&value);
    if (!range)
    return *this == *range;
}

void
Range::merge(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

size_t
Range::memoryUsage() const
{
    return sizeof(Range);
}

std::string
Range::toString() const
{
    std::stringstream ss;
    ss << "Integer::Range: ";
    if (mEmpty)
        ss << "empty";
    else
    {
        ss << "{" << std::endl;
        ss << "    from:" << (mFromInfinity ? "-infinity" : Canal::toString(mFrom)) << std::endl;
        ss << "    to:" << (mToInfinity ? "infinity" : Canal::toString(mTo)) << std::endl;
        ss << "}";
    }
    return ss.str();
}

float
Range::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Range::isBottom() const
{
    return mEmpty;
}

void
Range::setTop()
{
    mFromInfinity = mToInfinity = true;
    mEmpty = false;
}

} // namespace Integer
} // namespace Canal
