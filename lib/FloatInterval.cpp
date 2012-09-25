#include "FloatInterval.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Float {

Interval::Interval(const Environment &environment,
                   const llvm::fltSemantics &semantics)
    : Domain(environment),
      mFrom(semantics),
      mTo(semantics),
      mEmpty(true),
      mTop(false)
{
}

int
Interval::compare(const Interval &value,
                  llvm::CmpInst::Predicate predicate) const
{
    if (isTop() || value.isTop())
        return 2;

    if (isBottom() || value.isBottom())
        return -1;
    switch (predicate) { //Handle ordered and false/true
    //   Opcode                        U L G E  Intuitive operation
    case llvm::CmpInst::FCMP_FALSE: // 0 0 0 0  always false (always folded)
        return 0;
    case llvm::CmpInst::FCMP_OEQ:   // 0 0 0 1  ordered and equal
    case llvm::CmpInst::FCMP_OGT:   // 0 0 1 0  ordered and greater than
    case llvm::CmpInst::FCMP_OGE:   // 0 0 1 1  ordered and greater than or equal
    case llvm::CmpInst::FCMP_OLT:   // 0 1 0 0  ordered and less than
    case llvm::CmpInst::FCMP_OLE:   // 0 1 0 1  ordered and less than or equal
    case llvm::CmpInst::FCMP_ONE:   // 0 1 1 0  ordered and operands are unequal
        if (this->isNaN() || value.isNaN()) return 0; //Is unordered
        break;
    case llvm::CmpInst::FCMP_ORD:   // 0 1 1 1  ordered (no nans)
        return (this->isNaN() || value.isNaN() ? 0 : 1);
    case llvm::CmpInst::FCMP_UNO:   // 1 0 0 0  unordered: isnan(X) | isnan(Y)
        return (this->isNaN() || value.isNaN() ? 1 : 0);
    case llvm::CmpInst::FCMP_UEQ:   // 1 0 0 1  unordered or equal
    case llvm::CmpInst::FCMP_UGT:   // 1 0 1 0  unordered or greater than
    case llvm::CmpInst::FCMP_UGE:   // 1 0 1 1  unordered, greater than, or equal
    case llvm::CmpInst::FCMP_ULT:   // 1 1 0 0  unordered or less than
    case llvm::CmpInst::FCMP_ULE:   // 1 1 0 1  unordered, less than, or equal
    case llvm::CmpInst::FCMP_UNE:   // 1 1 1 0  unordered or not equal
        if (this->isNaN() || value.isNaN()) return 1; //Is unordered
        break;
    case llvm::CmpInst::FCMP_TRUE:  // 1 1 1 1  always true (always folded)
        return 1;
        break;
    default:
        CANAL_DIE();
    }

    llvm::APFloat::cmpResult res;

    switch (predicate)
    {
    // For more info, see Canal::Integer::Interval::icmp implementation.
    // Ordered means that neither operand is a QNAN while
    // unordered means that either operand may be a QNAN.
    //
    //   Opcode                        U L G E  Intuitive operation
    case llvm::CmpInst::FCMP_OEQ:   // 0 0 0 1  ordered and equal
    case llvm::CmpInst::FCMP_UEQ:   // 1 0 0 1  unordered or equal
        if (this == &value || (this->isSingleValue() && this->operator ==(value)))
            return 1;
        else if (this->intersects(value)) return 2;
        else return 0;
        break;
    case llvm::CmpInst::FCMP_OGT:   // 0 0 1 0  ordered and greater than
    case llvm::CmpInst::FCMP_UGT:   // 1 0 1 0  unordered or greater than
        res = this->mFrom.compare(value.mTo);
        if (res == llvm::APFloat::cmpGreaterThan) return 1;
        else if (this->intersects(value)) return 2;
        else return 0;
        break;
    case llvm::CmpInst::FCMP_OGE:   // 0 0 1 1  ordered and greater than or equal
    case llvm::CmpInst::FCMP_UGE:   // 1 0 1 1  unordered, greater than, or equal
        res = this->mFrom.compare(value.mTo);
        if (res == llvm::APFloat::cmpGreaterThan || res == llvm::APFloat::cmpEqual) return 1;
        else if (this->intersects(value)) return 2;
        else return 0;
        break;
    case llvm::CmpInst::FCMP_OLT:   // 0 1 0 0  ordered and less than
    case llvm::CmpInst::FCMP_ULT:   // 1 1 0 0  unordered or less than
        res = this->mTo.compare(value.mFrom);
        if (res == llvm::APFloat::cmpLessThan) return 1;
        else if (this->intersects(value)) return 2;
        else return 0;
        break;
    case llvm::CmpInst::FCMP_OLE:   // 0 1 0 1  ordered and less than or equal
    case llvm::CmpInst::FCMP_ULE:   // 1 1 0 1  unordered, less than, or equal
        res = this->mTo.compare(value.mFrom);
        if (res == llvm::APFloat::cmpLessThan || res == llvm::APFloat::cmpEqual) return 1;
        else if (this->intersects(value)) return 2;
        else return 0;
        break;
    case llvm::CmpInst::FCMP_ONE:   // 0 1 1 0  ordered and operands are unequal
    case llvm::CmpInst::FCMP_UNE:   // 1 1 1 0  unordered or not equal
        if (!this->intersects(value)) return 1;
        else if (!(this == &value || (this->isSingleValue() && this->operator ==(value))))
            return 2;
        else return 0;
        break;
    }
}

const llvm::fltSemantics &
Interval::getSemantics() const
{
    return mFrom.getSemantics();
}

bool
Interval::isSingleValue() const
{
    if (isBottom() || isTop())
        return false;

    return mFrom.compare(mTo) == llvm::APFloat::cmpEqual;
}

bool
Interval::intersects(const Interval &value) const
{
    llvm::APFloat::cmpResult res;
    res = this->getMax().compare(value.getMin());
    if (res == llvm::APFloat::cmpEqual || res == llvm::APFloat::cmpGreaterThan) {
        res = this->getMin().compare(value.getMax());
        if (res == llvm::APFloat::cmpEqual || res == llvm::APFloat::cmpLessThan) {
            return true;
        }
    }
    return false;
}

llvm::APFloat
Interval::getMax() const
{
    return (mTop ? llvm::APFloat::getLargest(getSemantics(), false) : mTo);
}

llvm::APFloat
Interval::getMin() const
{
    return (mTop ? llvm::APFloat::getLargest(getSemantics(), true) : mFrom);
}


bool
Interval::isNaN() const
{
    return this->mFrom.isNaN() || this->mTo.isNaN();
}

Interval *
Interval::clone() const
{
    return new Interval(*this);
}

Interval *
Interval::cloneCleaned() const
{
    return new Interval(mEnvironment, getSemantics());
}

bool
Interval::operator==(const Domain& value) const
{
    const Interval *interval = dynCast<const Interval*>(&value);
    if (!interval)
        return false;
    if (mEmpty)
        return interval->mEmpty;
    if (isTop())
        return interval->isTop();

    return mFrom.compare(interval->mFrom) == llvm::APFloat::cmpEqual &&
        mTo.compare(interval->mTo) == llvm::APFloat::cmpEqual;
}

void
Interval::merge(const Domain &value)
{
    const Interval &interval = dynCast<const Interval&>(value);
    if (interval.mEmpty)
        return;

    mEmpty = false;

    if (interval.isTop())
    {
        mTop = true;
        return;
    }

    if (mFrom.compare(interval.mFrom) == llvm::APFloat::cmpGreaterThan)
        mFrom = interval.mFrom;

    if (mTo.compare(interval.mTo) == llvm::APFloat::cmpLessThan)
        mTo = interval.mTo;
}

size_t
Interval::memoryUsage() const
{
    return sizeof(Interval);
}

std::string
Interval::toString() const
{
    std::stringstream ss;
    ss << "floatInterval ";
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

bool
Interval::matchesString(const std::string &text,
                     std::string &rationale) const
{
    CANAL_NOT_IMPLEMENTED();
}

float
Interval::accuracy() const
{
    if (mEmpty)
        return 1.0f;

    if (mTop)
        return 0.0f;

    llvm::APFloat divisor = llvm::APFloat::getLargest(
        getSemantics(), /*negative=*/false);
    llvm::APFloat::opStatus status = divisor.subtract(
        llvm::APFloat::getLargest(getSemantics(), /*negative=*/true),
        llvm::APFloat::rmNearestTiesToEven);

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
Interval::isBottom() const
{
    return mEmpty;
}

void
Interval::setBottom()
{
    mEmpty = true;
}

bool
Interval::isTop() const
{
    return !mEmpty && mTop;
}

void
Interval::setTop()
{
    mEmpty = false;
    mTop = true;
}


} // namespace Float
} // namespace Canal

