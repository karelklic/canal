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

int
Range::compare(const Range &value,
               llvm::CmpInst::Predicate predicate) const
{
    if (isTop() || value.isTop())
        return 2;

    if (isBottom() || value.isBottom())
        return -1;

    switch (predicate)
    {
    // Ordered means that neither operand is a QNAN while
    // unordered means that either operand may be a QNAN.
    //
    //   Opcode                        U L G E  Intuitive operation
    case llvm::CmpInst::FCMP_FALSE: // 0 0 0 0  always false (always folded)
        break;
    case llvm::CmpInst::FCMP_OEQ:   // 0 0 0 1  ordered and equal
        break;
    case llvm::CmpInst::FCMP_OGT:   // 0 0 1 0  ordered and greater than
        break;
    case llvm::CmpInst::FCMP_OGE:   // 0 0 1 1  ordered and greater than or equal
        break;
    case llvm::CmpInst::FCMP_OLT:   // 0 1 0 0  ordered and less than
        break;
    case llvm::CmpInst::FCMP_OLE:   // 0 1 0 1  ordered and less than or equal
        break;
    case llvm::CmpInst::FCMP_ONE:   // 0 1 1 0  ordered and operands are unequal
        break;
    case llvm::CmpInst::FCMP_ORD:   // 0 1 1 1  ordered (no nans)
        break;
    case llvm::CmpInst::FCMP_UNO:   // 1 0 0 0  unordered: isnan(X) | isnan(Y)
        break;
    case llvm::CmpInst::FCMP_UEQ:   // 1 0 0 1  unordered or equal
        break;
    case llvm::CmpInst::FCMP_UGT:   // 1 0 1 0  unordered or greater than
        break;
    case llvm::CmpInst::FCMP_UGE:   // 1 0 1 1  unordered, greater than, or equal
        break;
    case llvm::CmpInst::FCMP_ULT:   // 1 1 0 0  unordered or less than
        break;
    case llvm::CmpInst::FCMP_ULE:   // 1 1 0 1  unordered, less than, or equal
        break;
    case llvm::CmpInst::FCMP_UNE:   // 1 1 1 0  unordered or not equal
        break;
    case llvm::CmpInst::FCMP_TRUE:  // 1 1 1 1  always true (always folded)
        break;
    default:
        CANAL_DIE();
    }

    CANAL_NOT_IMPLEMENTED();
}

Range *
Range::clone() const
{
    return new Range(*this);
}

Range *
Range::cloneCleaned() const
{
    return new Range(getSemantics());
}

bool
Range::operator==(const Value& value) const
{
    const Range *range = dynCast<const Range*>(&value);
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
    const Range &range = dynCast<const Range&>(value);
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

    llvm::APFloat divisor = llvm::APFloat::getLargest(getSemantics(), /*negative=*/false);
    llvm::APFloat::opStatus status = divisor.subtract(llvm::APFloat::getLargest(getSemantics(), /*negative=*/true), llvm::APFloat::rmNearestTiesToEven);
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

