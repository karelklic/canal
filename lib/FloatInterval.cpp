#include "FloatInterval.h"
#include "IntegerContainer.h"
#include "IntegerInterval.h"
#include "Utils.h"
#include "FloatUtils.h"
#include "Environment.h"
#include "Constructors.h"
#include "IntegerUtils.h"

#define ROUNDING_MODE llvm::APFloat::rmNearestTiesToEven

namespace Canal {
namespace Float {

Interval::Interval(const Environment &environment,
                   const llvm::fltSemantics &semantics)
    : Domain(environment),
      mEmpty(true),
      mTop(false),
      mFrom(semantics),
      mTo(semantics)
{
}

Interval::Interval(const Environment &environment,
                   const llvm::APFloat &constant)
    : Domain(environment),
      mEmpty(false),
      mTop(false),
      mFrom(constant),
      mTo(constant)
{
}

Interval::Interval(const Interval &value)
    : Domain(value),
      mEmpty(value.mEmpty),
      mTop(value.mTop),
      mFrom(value.mFrom),
      mTo(value.mTo)
{
}

Interval::Interval(const Environment &environment,
                   const llvm::APFloat &from,
                   const llvm::APFloat &to)
    : Domain(environment),
      mEmpty(false),
      mTop(false),
      mFrom(from),
      mTo(to)
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

    // Handle ordered and false/true
    switch (predicate)
    {
    //   Opcode                        U L G E  Intuitive operation
    case llvm::CmpInst::FCMP_FALSE: // 0 0 0 0  always false (always folded)
        return 0;
    case llvm::CmpInst::FCMP_OEQ:   // 0 0 0 1  ordered and equal
    case llvm::CmpInst::FCMP_OGT:   // 0 0 1 0  ordered and greater than
    case llvm::CmpInst::FCMP_OGE:   // 0 0 1 1  ordered and greater than or equal
    case llvm::CmpInst::FCMP_OLT:   // 0 1 0 0  ordered and less than
    case llvm::CmpInst::FCMP_OLE:   // 0 1 0 1  ordered and less than or equal
    case llvm::CmpInst::FCMP_ONE:   // 0 1 1 0  ordered and operands are unequal
        if (isNaN() || value.isNaN())
            return 0; // Is unordered

        break;
    case llvm::CmpInst::FCMP_ORD:   // 0 1 1 1  ordered (no nans)
        return (isNaN() || value.isNaN() ? 0 : 1);
    case llvm::CmpInst::FCMP_UNO:   // 1 0 0 0  unordered: isnan(X) | isnan(Y)
        return (isNaN() || value.isNaN() ? 1 : 0);
    case llvm::CmpInst::FCMP_UEQ:   // 1 0 0 1  unordered or equal
    case llvm::CmpInst::FCMP_UGT:   // 1 0 1 0  unordered or greater than
    case llvm::CmpInst::FCMP_UGE:   // 1 0 1 1  unordered, greater than, or equal
    case llvm::CmpInst::FCMP_ULT:   // 1 1 0 0  unordered or less than
    case llvm::CmpInst::FCMP_ULE:   // 1 1 0 1  unordered, less than, or equal
    case llvm::CmpInst::FCMP_UNE:   // 1 1 1 0  unordered or not equal
        if (isNaN() || value.isNaN())
            return 1; // Is unordered

        break;
    case llvm::CmpInst::FCMP_TRUE:  // 1 1 1 1  always true (always folded)
        return 1;
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
        if (isConstant() && *this == value)
            return 1;
        else if (intersects(value))
            return 2;
        else
            return 0;

        break;
    case llvm::CmpInst::FCMP_OGT:   // 0 0 1 0  ordered and greater than
    case llvm::CmpInst::FCMP_UGT:   // 1 0 1 0  unordered or greater than
        res = mFrom.compare(value.mTo);
        if (res == llvm::APFloat::cmpGreaterThan)
            return 1;
        else if (intersects(value))
            return 2;
        else
            return 0;

        break;
    case llvm::CmpInst::FCMP_OGE:   // 0 0 1 1  ordered and greater than or equal
    case llvm::CmpInst::FCMP_UGE:   // 1 0 1 1  unordered, greater than, or equal
        res = mFrom.compare(value.mTo);
        if (res == llvm::APFloat::cmpGreaterThan ||
            res == llvm::APFloat::cmpEqual)
        {
            return 1;
        }
        else if (intersects(value))
            return 2;
        else
            return 0;

        break;
    case llvm::CmpInst::FCMP_OLT:   // 0 1 0 0  ordered and less than
    case llvm::CmpInst::FCMP_ULT:   // 1 1 0 0  unordered or less than
        res = mTo.compare(value.mFrom);
        if (res == llvm::APFloat::cmpLessThan)
            return 1;
        else if (intersects(value))
            return 2;
        else
            return 0;

        break;
    case llvm::CmpInst::FCMP_OLE:   // 0 1 0 1  ordered and less than or equal
    case llvm::CmpInst::FCMP_ULE:   // 1 1 0 1  unordered, less than, or equal
        res = mTo.compare(value.mFrom);
        if (res == llvm::APFloat::cmpLessThan ||
            res == llvm::APFloat::cmpEqual)
        {
            return 1;
        }
        else if (intersects(value))
            return 2;
        else
            return 0;

        break;
    case llvm::CmpInst::FCMP_ONE:   // 0 1 1 0  ordered and operands are unequal
    case llvm::CmpInst::FCMP_UNE:   // 1 1 1 0  unordered or not equal
        if (!intersects(value))
            return 1;
        else if (!(isConstant() && *this == value))
            return 2;
        else
            return 0;

        break;
    default:
        CANAL_DIE();
    }

    CANAL_DIE_MSG("Unexpectedly failed to compare float intervals.");
}

const llvm::fltSemantics &
Interval::getSemantics() const
{
    return mFrom.getSemantics();
}

bool
Interval::isConstant() const
{
    if (isBottom() || isTop())
        return false;

    return mFrom.compare(mTo) == llvm::APFloat::cmpEqual;
}

bool
Interval::intersects(const Interval &value) const
{
    if (isBottom() || value.isBottom())
        return false;

    llvm::APFloat::cmpResult res;
    res = getMax().compare(value.getMin());
    if (res == llvm::APFloat::cmpEqual ||
        res == llvm::APFloat::cmpGreaterThan)
    {
        res = getMin().compare(value.getMax());
        if (res == llvm::APFloat::cmpEqual ||
            res == llvm::APFloat::cmpLessThan)
        {
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
    return !mEmpty && !mTop && (mFrom.isNaN() || mTo.isNaN());
}

Interval *
Interval::clone() const
{
    return new Interval(*this);
}

size_t
Interval::memoryUsage() const
{
    return sizeof(Interval);
}

std::string
Interval::toString() const
{
    StringStream ss;
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

    ss << "\n";
    return ss.str();
}

void
Interval::setZero(const llvm::Value *place)
{
    mEmpty = false;
    mFrom = llvm::APFloat::getZero(mFrom.getSemantics());
    mTo = llvm::APFloat::getZero(mTo.getSemantics());
}

bool
Interval::operator==(const Domain& value) const
{
    if (this == &value)
        return true;

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

bool
Interval::operator<(const Domain& value) const
{
    CANAL_NOT_IMPLEMENTED();
}

Interval &
Interval::join(const Domain &value)
{
    if (isTop())
        return *this;

    if (value.isBottom())
        return *this;

    if (value.isTop())
    {
        setTop();
        return *this;
    }

    const Interval &interval = dynCast<const Interval&>(value);

    if (isBottom())
    {
        mFrom = interval.mFrom;
        mTo = interval.mTo;
    }
    else
    {
        if (mFrom.compare(interval.mFrom) == llvm::APFloat::cmpGreaterThan)
            mFrom = interval.mFrom;
        if (mEmpty || mTo.compare(interval.mTo) == llvm::APFloat::cmpLessThan)
            mTo = interval.mTo;
    }

    mEmpty = false;
    return *this;
}

Interval &
Interval::meet(const Domain &value)
{
    if (isBottom())
        return *this;

    if (value.isTop())
        return *this;

    if (value.isBottom())
    {
        setBottom();
        return *this;
    }

    const Interval &interval = dynCast<const Interval&>(value);
    if (isTop())
    {
        mFrom = interval.mFrom;
        mTo = interval.mTo;
    }
    else if (intersects(interval))
    {
        if (mFrom.compare(interval.mFrom) == llvm::APFloat::cmpLessThan)
            mFrom = interval.mFrom;

        if (mTo.compare(interval.mTo) == llvm::APFloat::cmpGreaterThan)
            mTo = interval.mTo;
    }
    else
        mEmpty = true;

    mTop = false;
    return *this;
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
    mTop = false;
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

// See IntegerInterval.cpp minMax
static bool
minMax(llvm::APFloat& min,
       llvm::APFloat &max,
       const llvm::APFloat& fromFrom,
       const llvm::APFloat& fromTo,
       const llvm::APFloat& toFrom,
       const llvm::APFloat& toTo)
{
    llvm::APFloat::cmpResult res;

#define CMP(x, y)                           \
    res = x.compare(y);                     \
    if (res == llvm::APFloat::cmpUnordered) \
        return true;

#define CMPRES res == llvm::APFloat::cmpLessThan

    CMP(toTo, toFrom);
    if (CMPRES)
    { // toTo < toFrom
        CMP(toTo, fromTo);
        if (CMPRES)
        { // toTo < (toFrom, fromTo)
            CMP(toTo, fromFrom);
            if (CMPRES)
            { // toTo < (toFrom, fromTo, fromFrom)
                min = toTo;
                CMP(toFrom, fromTo);
                if (CMPRES)
                { // toTo < (fromFrom, toFrom < fromTo)
                    CMP(fromFrom, fromTo);
                    if (CMPRES)
                        max = fromTo; // toTo < (fromFrom, toFrom) < fromTo
                    else
                        max = fromFrom; // toTo < toFrom < fromTo <= fromFrom
                }
            }
            else
            { // fromFrom <= toTo < (toFrom, fromTo)
                min = fromFrom;
                CMP(toFrom, fromTo);
                if (CMPRES)
                    max = fromTo; // fromFrom <= toTo < toFrom < fromTo;
                else
                    max = toFrom; // fromFrom <= toTo < fromTo <= toFrom;
            }
        }
        else
        { // fromTo <= toTo < toFrom
            CMP(fromTo, fromFrom);
            if (CMPRES)
            { // fromTo <= (fromFrom, toTo < toFrom)
                min = fromTo;
                CMP(fromFrom, toFrom);
                if (CMPRES)
                    max = toFrom; // fromTo <= (fromFrom, toTo) < toFrom
                else
                    max = fromFrom; // fromTo <= toTo < toFrom <= fromFrom;
            }
            else
            { // fromFrom <= fromTo <= toTo < toFrom
                min = fromFrom;
                max = toFrom;
            }
        }
    }
    else
    { // toFrom <= toTo
        CMP(toFrom, fromTo);
        if (CMPRES)
        { // toFrom <= (toTo, fromTo)
            CMP(toFrom, fromFrom);
            if (CMPRES)
            { // toFrom <= (toTo, fromTo, fromFrom)
                min = toFrom;
                CMP(toTo, fromTo);
                if (CMPRES)
                { // toFrom <= (fromFrom, toTo < fromTo)
                    CMP(fromFrom, fromTo);
                    if (CMPRES)
                        max = fromTo; // toFrom <= (fromFrom, toTo) < fromTo
                    else
                        max = fromFrom; // toFrom <= toTo < fromTo <= fromFrom
                }
                else
                { // toFrom <= (fromFrom, fromTo <= toTo)
                    CMP(fromFrom, toTo);
                    if (CMPRES)
                        max = toTo; // toFrom <= (fromTo, fromFrom) <= toTo
                    else
                        max = fromFrom; // toFrom <= fromTo <= toTo <= fromFrom
                }
            }
            else
            { // fromFrom <= toFrom <= (toTo, fromTo)
                min = fromFrom;
                CMP(toTo, fromTo);
                if (CMPRES)
                    max = fromTo; // fromFrom <= toFrom <= toTo < fromTo
                else
                    max = toTo; // fromFrom <= toFrom <= fromTo <= toTo
            }
        }
        else
        { // fromTo <= toFrom <= toTo
            CMP(fromTo, fromFrom);
            if (CMPRES)
            { // fromTo <= (fromFrom, toFrom <= toTo)
                min = fromTo;
                CMP(fromFrom, toTo);
                if (CMPRES)
                    max = toTo; // fromTo <= (fromFrom, toFrom) <= toTo
                else
                    max = fromFrom; // fromTo <= toFrom <= toTo <= fromFrom
            }
            else
            { // fromFrom <= fromTo <= toFrom <= toTo
                min = fromFrom;
                max = toTo;
            }
        }
    }
#undef CMP
#undef CMPRES
    return false;
}

#define OVERFLOW_TO_TOP(op)        \
    if (op != llvm::APFloat::opOK) \
    {                              \
        setTop();                  \
        return *this;              \
    }

Interval &
Interval::fadd(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return *this;

    mTop = (aa.mTop || bb.mTop);
    if (!mTop)
    {
        mFrom = aa.mFrom;
        OVERFLOW_TO_TOP(mFrom.add(bb.mFrom, ROUNDING_MODE));

        mTo = aa.mTo;
        OVERFLOW_TO_TOP(mTo.add(bb.mFrom, ROUNDING_MODE));
    }

    return *this;
}

Interval &
Interval::fsub(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return *this;

    mTop = (aa.mTop || bb.mTop);
    if (!mTop)
    {
        mFrom = aa.mFrom;
        OVERFLOW_TO_TOP(mFrom.subtract(bb.mFrom, ROUNDING_MODE));

        mTo = aa.mTo;
        OVERFLOW_TO_TOP(mTo.subtract(bb.mFrom, ROUNDING_MODE));
    }

    return *this;
}

Interval &
Interval::fmul(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return *this;

    mTop = (aa.mTop || bb.mTop);
    if (!mTop)
    {
        llvm::APFloat fromFrom(aa.mFrom), fromTo(aa.mFrom),
                toFrom(aa.mTo), toTo(aa.mTo);

        OVERFLOW_TO_TOP(fromFrom.multiply(bb.mFrom, ROUNDING_MODE));
        OVERFLOW_TO_TOP(fromTo.multiply(bb.mTo, ROUNDING_MODE));
        OVERFLOW_TO_TOP(toFrom.multiply(bb.mFrom, ROUNDING_MODE));
        OVERFLOW_TO_TOP(toTo.multiply(bb.mTo, ROUNDING_MODE));
        mTop = minMax(mFrom, mTo,
                      fromFrom, fromTo,
                      toFrom, toTo);
    }

    return *this;
}

Interval &
Interval::fdiv(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return *this;

    mTop = (aa.mTop || bb.mTop);
    if (!mTop)
    {
        llvm::APFloat fromFrom(aa.mFrom), fromTo(aa.mFrom),
                toFrom(aa.mTo), toTo(aa.mTo);

        OVERFLOW_TO_TOP(fromFrom.divide(bb.mFrom, ROUNDING_MODE));
        OVERFLOW_TO_TOP(fromTo.divide(bb.mTo, ROUNDING_MODE));
        OVERFLOW_TO_TOP(toFrom.divide(bb.mFrom, ROUNDING_MODE));
        OVERFLOW_TO_TOP(toTo.divide(bb.mTo, ROUNDING_MODE));
        mTop = minMax(mFrom, mTo,
                      fromFrom, fromTo,
                      toFrom, toTo);
    }

    return *this;
}

Interval &
Interval::frem(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return *this;

    mTop = (aa.mTop || bb.mTop);
    setTop();
    return *this;
}

#undef OVERFLOW_TO_TOP

Interval &
Interval::uitofp(const Domain &value)
{
    llvm::APInt min, max;
    if (!Integer::Utils::unsignedMin(value, min) ||
        !Integer::Utils::unsignedMax(value, max))
    {
        setTop();
        return *this;
    }

    mFrom = llvm::APFloat(min);
    mTo = llvm::APFloat(max);
    return *this;
}

Interval &
Interval::sitofp(const Domain &value)
{
    llvm::APInt min, max;
    if (!Integer::Utils::signedMin(value, min) ||
        !Integer::Utils::signedMax(value, max))
    {
        setTop();
        return *this;
    }

    mFrom = llvm::APFloat(min);
    mTo = llvm::APFloat(max);
    return *this;
}

const llvm::Type &
Interval::getValueType() const
{
    return Utils::getType(mFrom.getSemantics(),
                          mEnvironment.getContext());
}

Domain *
Interval::getValueCell(size_t offset) const
{
    Domain *cell = mEnvironment.getConstructors().createInteger(8);
    cell->setTop();
    return cell;
}

void
Interval::mergeValueCell(size_t offset, const Domain &value)
{
    setTop();
}

} // namespace Float
} // namespace Canal

