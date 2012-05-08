#include "IntegerRange.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Range::Range(unsigned numBits)
    : mEmpty(true),
      mTop(false),
      mFrom(numBits, 0),
      mTo(numBits, 0)
{
}

Range::Range(const llvm::APInt &constant)
    : mEmpty(false),
      mTop(false),
      mFrom(constant),
      mTo(constant)
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
        return false;
    if (mEmpty)
        return range->mEmpty;
    if (mTop)
        return range->mTop;
    return mFrom == range->mFrom && mTo == range->mTo;
}

void
Range::merge(const Value &value)
{
    const Range &range = dynamic_cast<const Range&>(value);
    if (range.mEmpty)
        return;

    if (range.mTop)
    {
        mEmpty = false;
        mTop = true;
        return;
    }

    if (!mFrom.sle(range.mFrom))
        mFrom = range.mFrom;
    if (!mTo.sgt(range.mTo))
        mTo = range.mTo;
}

size_t
Range::memoryUsage() const
{
    return sizeof(Range);
}

std::string
Range::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Integer::Range: ";
    if (mEmpty)
        ss << "empty";
    else
    {
        ss << "{" << std::endl;
        ss << "    from:" << (mTop ? "-infinity" : Canal::toString(mFrom)) << std::endl;
        ss << "    to:" << (mTop ? "infinity" : Canal::toString(mTo)) << std::endl;
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
    mTop = true;
    mEmpty = false;
}

} // namespace Integer
} // namespace Canal
