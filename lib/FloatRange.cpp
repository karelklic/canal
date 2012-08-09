#include "FloatRange.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Float {

Range::Range(const llvm::fltSemantics &semantics)
    : mFrom(semantics),
      mTo(semantics),
      mEmpty(true),
      mTop(false)
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
    if (isTop())
        return range->isTop();

    return mFrom.compare(range->mFrom) == llvm::APFloat::cmpEqual &&
        mTo.compare(range->mTo) == llvm::APFloat::cmpEqual;
}

void
Range::merge(const Value &value)
{
    const Range &range = dynamic_cast<const Range&>(value);
    if (range.mEmpty)
        return;

    mEmpty = false;

    if (range.isTop())
    {
        mTop = true;
        return;
    }

    if (mFrom.compare(range.mFrom) == llvm::APFloat::cmpGreaterThan)
        mFrom = range.mFrom;

    if (mTo.compare(range.mTo) == llvm::APFloat::cmpLessThan)
        mTo = range.mTo;
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
    ss << "floatRange ";
    if (mEmpty)
        ss << "empty";
    else if (mTop)
        ss << "-infinity to infinity";
    else
    {
        ss << mFrom.convertToDouble() << " to " <<
            mTo.convertToDouble();
    }

    ss << std::endl;
    return ss.str();
}

float
Range::accuracy() const
{
    if (mEmpty)
        return 1.0f;

    if (mTop)
        return 0.0f;

    llvm::APFloat divisor = llvm::APFloat::getLargest(mFrom.getSemantics(), /*negative=*/false);
    llvm::APFloat::opStatus status = divisor.subtract(llvm::APFloat::getLargest(mFrom.getSemantics(), /*negative=*/true), llvm::APFloat::rmNearestTiesToEven);
    CANAL_ASSERT(status == llvm::APFloat::opOK);

    llvm::APFloat dividend = mTo;
    status = dividend.subtract(mFrom, llvm::APFloat::rmNearestTiesToEven);
    CANAL_ASSERT(status == llvm::APFloat::opOK);

    llvm::APFloat coverage = dividend;
    status = coverage.divide(divisor, llvm::APFloat::rmNearestTiesToEven);
    CANAL_ASSERT(status == llvm::APFloat::opOK);

    float result = 1.0f - coverage.convertToFloat();
    return result > 1.0f ? 1.0f : (result < 0.0f ? 0.0f : result);
}

bool
Range::isBottom() const
{
    return mEmpty;
}

void
Range::setBottom()
{
    mEmpty = true;
}

bool
Range::isTop() const
{
    return !mEmpty && mTop;
}

void
Range::setTop()
{
    mEmpty = false;
    mTop = true;
}


} // namespace Float
} // namespace Canal

