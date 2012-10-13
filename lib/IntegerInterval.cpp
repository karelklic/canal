#include "IntegerInterval.h"
#include "Utils.h"
#include "APIntUtils.h"
#include "FloatInterval.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Interval::Interval(const Environment &environment,
             unsigned bitWidth)
    : Domain(environment),
      mEmpty(true),
      mSignedTop(false),
      mSignedFrom(bitWidth, 0),
      mSignedTo(bitWidth, 0),
      mUnsignedTop(false),
      mUnsignedFrom(bitWidth, 0),
      mUnsignedTo(bitWidth, 0)
{
}

Interval::Interval(const Environment &environment,
             const llvm::APInt &constant)
    : Domain(environment),
      mEmpty(false),
      mSignedTop(false),
      mSignedFrom(constant),
      mSignedTo(constant),
      mUnsignedTop(false),
      mUnsignedFrom(constant),
      mUnsignedTo(constant)
{
}

bool
Interval::signedMin(llvm::APInt &result) const
{
    if (mEmpty)
        return false;

    if (mSignedTop)
        result = llvm::APInt::getSignedMinValue(mSignedFrom.getBitWidth());
    else
        result = mSignedFrom;

    return true;
}

bool
Interval::signedMax(llvm::APInt &result) const
{
    if (mEmpty)
        return false;

    if (mSignedTop)
        result = llvm::APInt::getSignedMaxValue(mSignedTo.getBitWidth());
    else
        result = mSignedTo;

    return true;
}

bool
Interval::unsignedMin(llvm::APInt &result) const
{
    if (mEmpty)
        return false;

    if (mUnsignedTop)
        result = llvm::APInt::getMinValue(mUnsignedFrom.getBitWidth());
    else
        result = mUnsignedFrom;

    return true;
}

bool
Interval::unsignedMax(llvm::APInt &result) const
{
    if (mEmpty)
        return false;

    if (mUnsignedTop)
        result = llvm::APInt::getMaxValue(mUnsignedTo.getBitWidth());
    else
        result = mUnsignedTo;

    return true;
}

bool
Interval::isSingleValue() const
{
    return isSignedSingleValue() && isUnsignedSingleValue();
}

bool
Interval::isSignedSingleValue() const
{
    if (isBottom() || mSignedTop)
        return false;

    return mSignedFrom == mSignedTo;
}

bool
Interval::isUnsignedSingleValue() const
{
    if (isBottom() || mUnsignedTop)
        return false;

    return mUnsignedFrom == mUnsignedTo;
}

Interval *
Interval::clone() const
{
    return new Interval(*this);
}

Interval *
Interval::cloneCleaned() const
{
    return new Interval(mEnvironment, getBitWidth());
}

bool
Interval::operator==(const Domain& value) const
{
    const Interval *interval = dynCast<const Interval*>(&value);
    if (!interval)
        return false;

    if (getBitWidth() != interval->getBitWidth())
        return false;

    if (mEmpty || interval->mEmpty)
        return mEmpty == interval->mEmpty;
    if (mSignedTop ^ interval->mSignedTop || mUnsignedTop ^ interval->mUnsignedTop)
        return false;

    if (!mSignedTop && (mSignedFrom != interval->mSignedFrom ||
                        mSignedTo != interval->mSignedTo))
    {
        return false;
    }

    if (!mUnsignedTop && (mUnsignedFrom != interval->mUnsignedFrom ||
                          mUnsignedTo != interval->mUnsignedTo))
    {
        return false;
    }

    return true;
}

void
Interval::merge(const Domain &value)
{
    const Interval &interval = dynCast<const Interval&>(value);
    if (interval.mEmpty)
        return;

    if (mEmpty)
    {
        mEmpty = false;
        mSignedTop = interval.mSignedTop;
        mSignedFrom = interval.mSignedFrom;
        mSignedTo = interval.mSignedTo;
        mUnsignedTop = interval.mUnsignedTop;
        mUnsignedFrom = interval.mUnsignedFrom;
        mUnsignedTo = interval.mUnsignedTo;
        return;
    }

    if (!mSignedTop)
    {
        if (interval.mSignedTop)
            mSignedTop = true;
        else
        {
            if (!mSignedFrom.sle(interval.mSignedFrom))
                mSignedFrom = interval.mSignedFrom;
            if (!mSignedTo.sge(interval.mSignedTo))
                mSignedTo = interval.mSignedTo;
        }
    }

    if (!mUnsignedTop)
    {
        if (interval.mUnsignedTop)
            mUnsignedTop = true;
        else
        {
            if (!mUnsignedFrom.ule(interval.mUnsignedFrom))
                mUnsignedFrom = interval.mUnsignedFrom;
            if (!mUnsignedTo.uge(interval.mUnsignedTo))
                mUnsignedTo = interval.mUnsignedTo;
        }
    }
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
    ss << "interval";
    if (mEmpty)
        ss << " empty" << std::endl;
    else
    {
        llvm::APInt sMin, sMax, uMin, uMax;
        bool success = signedMin(sMin);
        CANAL_ASSERT(success);
        success = signedMax(sMax);
        CANAL_ASSERT(success);
        success = unsignedMin(uMin);
        CANAL_ASSERT(success);
        success = unsignedMax(uMax);
        CANAL_ASSERT(success);

        std::stringstream sign, unsign;
        sign << sMin.toString(10, true) << " to "
             << sMax.toString(10, true);
        if (mSignedTop)
            sign << " (top)";

        unsign << uMin.toString(10, false) << " to "
               << uMax.toString(10, false);
        if (mUnsignedTop)
            unsign << " (top)";

        if (sign.str() != unsign.str())
        {
            ss << std::endl;
            ss << "    signed " << sign.str() << std::endl;
            ss << "    unsigned " << unsign.str() << std::endl;
        }
        else
            ss << " " << sign.str() << std::endl;
    }

    return ss.str();
}

bool
Interval::matchesString(const std::string &text,
                     std::string &rationale) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Interval::add(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        mSignedFrom = APIntUtils::sadd_ov(aa.mSignedFrom,
                                          bb.mSignedFrom,
                                          mSignedTop);

        if (!mSignedTop)
            mSignedTo = APIntUtils::sadd_ov(aa.mSignedTo,
                                            bb.mSignedTo,
                                            mSignedTop);
    }

    mUnsignedTop = aa.mUnsignedTop || bb.mUnsignedTop;
    if (!mUnsignedTop)
    {
        mUnsignedFrom = APIntUtils::uadd_ov(aa.mUnsignedFrom,
                                            bb.mUnsignedFrom,
                                            mUnsignedTop);

        if (!mUnsignedTop)
            mUnsignedTo = APIntUtils::uadd_ov(aa.mUnsignedTo,
                                              bb.mUnsignedTo,
                                              mUnsignedTop);
    }
}

void
Interval::sub(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        mSignedFrom = APIntUtils::ssub_ov(aa.mSignedFrom,
                                          bb.mSignedTo,
                                          mSignedTop);

        if (!mSignedTop)
            mSignedTo = APIntUtils::ssub_ov(aa.mSignedTo,
                                            bb.mSignedFrom,
                                            mSignedTop);
    }

    mUnsignedTop = aa.mUnsignedTop || bb.mUnsignedTop;
    if (!mUnsignedTop)
    {
        mUnsignedFrom = APIntUtils::usub_ov(aa.mUnsignedFrom,
                                            bb.mUnsignedTo,
                                            mUnsignedTop);

        if (!mUnsignedTop)
            mUnsignedTo = APIntUtils::usub_ov(aa.mUnsignedTo,
                                              bb.mUnsignedFrom,
                                              mUnsignedTop);
    }
}

static void
minMax(bool isSigned,
       llvm::APInt &min,
       llvm::APInt &max,
       const llvm::APInt &toTo,
       const llvm::APInt &toFrom,
       const llvm::APInt &fromTo,
       const llvm::APInt &fromFrom)
{
    typedef bool(llvm::APInt::*LessThan)(const llvm::APInt&) const;
    LessThan lt(isSigned ? (LessThan)&llvm::APInt::slt : (LessThan)&llvm::APInt::ult);
    if ((toTo.*(lt))(toFrom))
    { // toTo < toFrom
        if ((toTo.*(lt))(fromTo))
        { // toTo < (toFrom, fromTo)
            if ((toTo.*(lt))(fromFrom))
            { // toTo < (toFrom, fromTo, fromFrom)
                min = toTo;
                if ((toFrom.*(lt))(fromTo))
                { // toTo < (fromFrom, toFrom < fromTo)
                    if ((fromFrom.*(lt))(fromTo))
                        max = fromTo; // toTo < (fromFrom, toFrom) < fromTo
                    else
                        max = fromFrom; // toTo < toFrom < fromTo <= fromFrom
                }
            }
            else
            { // fromFrom <= toTo < (toFrom, fromTo)
                min = fromFrom;
                if ((toFrom.*(lt))(fromTo))
                    max = fromTo; // fromFrom <= toTo < toFrom < fromTo;
                else
                    max = toFrom; // fromFrom <= toTo < fromTo <= toFrom;
            }
        }
        else
        { // fromTo <= toTo < toFrom
            if ((fromTo.*(lt))(fromFrom))
            { // fromTo <= (fromFrom, toTo < toFrom)
                min = fromTo;
                if ((fromFrom.*(lt))(toFrom))
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
        if ((toFrom.*(lt))(fromTo))
        { // toFrom <= (toTo, fromTo)
            if ((toFrom.*(lt))(fromFrom))
            { // toFrom <= (toTo, fromTo, fromFrom)
                min = toFrom;
                if ((toTo.*(lt))(fromTo))
                { // toFrom <= (fromFrom, toTo < fromTo)
                    if ((fromFrom.*(lt))(fromTo))
                        max = fromTo; // toFrom <= (fromFrom, toTo) < fromTo
                    else
                        max = fromFrom; // toFrom <= toTo < fromTo <= fromFrom
                }
                else
                { // toFrom <= (fromFrom, fromTo <= toTo)
                    if ((fromFrom.*(lt))(toTo))
                        max = toTo; // toFrom <= (fromTo, fromFrom) <= toTo
                    else
                        max = fromFrom; // toFrom <= fromTo <= toTo <= fromFrom
                }
            }
            else
            { // fromFrom <= toFrom <= (toTo, fromTo)
                min = fromFrom;
                if ((toTo.*(lt))(fromTo))
                    max = fromTo; // fromFrom <= toFrom <= toTo < fromTo
                else
                    max = toTo; // fromFrom <= toFrom <= fromTo <= toTo
            }
        }
        else
        { // fromTo <= toFrom <= toTo
            if ((fromTo.*(lt))(fromFrom))
            { // fromTo <= (fromFrom, toFrom <= toTo)
                min = fromTo;
                if ((fromFrom.*(lt))(toTo))
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
}

void
Interval::mul(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        llvm::APInt toTo = APIntUtils::smul_ov(aa.mSignedTo,
                                               bb.mSignedTo,
                                               mSignedTop);
        if (!mSignedTop)
        {
            llvm::APInt toFrom = APIntUtils::smul_ov(aa.mSignedTo,
                                                     bb.mSignedFrom,
                                                     mSignedTop);
            if (!mSignedTop)
            {
                llvm::APInt fromTo = APIntUtils::smul_ov(aa.mSignedFrom,
                                                         bb.mSignedTo,
                                                         mSignedTop);
                if (!mSignedTop)
                {
                    llvm::APInt fromFrom = APIntUtils::smul_ov(aa.mSignedFrom,
                                                               bb.mSignedFrom,
                                                               mSignedTop);
                    if (!mSignedTop)
                    {
                        minMax(/*signed=*/true,
                               mSignedFrom,
                               mSignedTo,
                               toTo,
                               toFrom,
                               fromTo,
                               fromFrom);
                    }
                }
            }
        }
    }

    mUnsignedTop = aa.mUnsignedTop || bb.mUnsignedTop;
    if (!mUnsignedTop)
    {
        llvm::APInt toTo = APIntUtils::umul_ov(aa.mUnsignedTo,
                                               bb.mUnsignedTo,
                                               mUnsignedTop);
        if (!mUnsignedTop)
        {
            llvm::APInt toFrom = APIntUtils::umul_ov(aa.mUnsignedTo,
                                                     bb.mUnsignedFrom,
                                                     mUnsignedTop);
            if (!mUnsignedTop)
            {
                llvm::APInt fromTo = APIntUtils::umul_ov(aa.mUnsignedFrom,
                                                         bb.mUnsignedTo,
                                                         mUnsignedTop);
                if (!mUnsignedTop)
                {
                    llvm::APInt fromFrom = APIntUtils::umul_ov(aa.mUnsignedFrom,
                                                               bb.mUnsignedFrom,
                                                               mUnsignedTop);
                    if (!mUnsignedTop)
                    {
                        minMax(/*signed=*/false,
                               mUnsignedFrom,
                               mUnsignedTo,
                               toTo,
                               toFrom,
                               fromTo,
                               fromFrom);
                    }
                }
            }
        }
    }
}

void
Interval::udiv(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = true;
    mUnsignedTop = aa.mUnsignedTop || bb.mUnsignedTop;
    if (!mUnsignedTop)
    {
        llvm::APInt toTo = aa.mUnsignedTo.udiv(bb.mUnsignedTo);
        llvm::APInt toFrom = aa.mUnsignedTo.udiv(bb.mUnsignedFrom);
        llvm::APInt fromTo = aa.mUnsignedFrom.udiv(bb.mUnsignedTo);
        llvm::APInt fromFrom = aa.mUnsignedFrom.udiv(bb.mUnsignedFrom);
        minMax(/*signed=*/false,
               mUnsignedFrom,
               mUnsignedTo,
               toTo,
               toFrom,
               fromTo,
               fromFrom);
    }
}

void
Interval::sdiv(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mUnsignedTop = true;
    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        llvm::APInt toTo = APIntUtils::sdiv_ov(aa.mSignedTo,
                                               bb.mSignedTo,
                                               mSignedTop);
        if (!mSignedTop)
        {
            llvm::APInt toFrom = APIntUtils::sdiv_ov(aa.mSignedTo,
                                                     bb.mSignedFrom,
                                                     mSignedTop);
            if (!mSignedTop)
            {
                llvm::APInt fromTo = APIntUtils::sdiv_ov(aa.mSignedFrom,
                                                         bb.mSignedTo,
                                                         mSignedTop);
                if (!mSignedTop)
                {
                    llvm::APInt fromFrom = APIntUtils::sdiv_ov(aa.mSignedFrom,
                                                               bb.mSignedFrom,
                                                               mSignedTop);
                    if (!mSignedTop)
                    {
                        minMax(/*signed=*/true,
                               mSignedFrom,
                               mSignedTo,
                               toTo,
                               toFrom,
                               fromTo,
                               fromFrom);
                    }
                }
            }
        }
    }
}

void
Interval::urem(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::srem(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::shl(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::lshr(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::ashr(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::and_(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::or_(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Interval::xor_(const Domain &a, const Domain &b)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

// Assumes that neither a nor b are empty or top
static bool
intersects(const Interval &a,
           const Interval &b,
           bool signed_,
           bool unsigned_)
{
    llvm::APInt i, j;
    if (signed_)
    {
        a.signedMax(i);
        b.signedMin(j);
        if (i.sge(j)) {
            b.signedMax(i);
            a.signedMin(j);
            if (i.sge(j)) return true;
        }
    }

    if (unsigned_)
    {
        a.unsignedMax(i);
        b.unsignedMin(j);
        if (i.uge(j)) {
            b.unsignedMax(i);
            a.unsignedMin(j);
            if (i.uge(j)) return true;
        }
    }

    return false;
}

void
Interval::icmp(const Domain &a, const Domain &b,
            llvm::CmpInst::Predicate predicate)
{
    const Interval &aa = dynCast<const Interval&>(a),
        &bb = dynCast<const Interval&>(b);

    if (aa.isTop() || bb.isTop())
    {
        // Result could be both true and false.
        setTop();
        return;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        // Result is undefined.  Fixpoint calculation need to
        // propagate the value of parameters.
        setBottom();
        return;
    }

    setBottom(); //Interval 0-0
    mEmpty = false;

    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If both intervals are equal, the result is 1.  If
        // interval intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b) {
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
            break;
        }

        //Signed equality
        if (aa.isSignedSingleValue() && bb.isSignedSingleValue() && aa.mSignedFrom == bb.mSignedFrom) {
            mSignedFrom = mSignedTo = 1;
        }
        else if (intersects(aa, bb, true, false)) mSignedTop = true;

        //Unsigned equality
        if (aa.isUnsignedSingleValue() && bb.isUnsignedSingleValue() && aa.mUnsignedFrom == bb.mUnsignedFrom) {
            mUnsignedFrom = mUnsignedTo = 1;
        }
        else if (intersects(aa, bb, false, true)) mUnsignedTop = true;

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If both intervals are equal, the result is 0.  If
        // interval intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b) {
            break;
        }

        //Signed inequality
        if (intersects(aa, bb, true, false)) mSignedTop = true;
        else if (!(aa.isSignedSingleValue() && bb.isSignedSingleValue() && aa.mSignedFrom == bb.mSignedFrom)) {
            mSignedFrom = mSignedTo = 1;
        }

        //Unsigned inequality
        if (intersects(aa, bb, false, true)) mUnsignedTop = true;
        else if (!(aa.isUnsignedSingleValue() && bb.isUnsignedSingleValue() && aa.mUnsignedFrom == bb.mUnsignedFrom)) {
            mUnsignedFrom = mUnsignedTo = 1;
        }
        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        // If the lowest element from the first interval is
        // unsigned greater than the largest element from the second
        // interval, the result is 1.  If the largest element from
        // the first interval is unsigned lower than the lowest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).

        if (aa.mUnsignedFrom.ugt(bb.mUnsignedTo))
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, false, true))
            this->setTop();

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the lowest element from the first interval is
        // unsigned greater or equal than the largest element from the second
        // interval, the result is 1.  If the largest element from
        // the first interval is unsigned lower than the lowest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedFrom.uge(bb.mUnsignedTo) || &a == &b)
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mUnsignedTo.ule(bb.mUnsignedFrom))
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first interval is
        // unsigned lower than the lowest element from the second
        // interval, the result is 1.  If the lowest element from
        // the first interval is unsigned larger than the largest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedTo.ult(bb.mUnsignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, false, true))
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first interval is
        // unsigned lower or equal the lowest element from the second
        // interval, the result is 1.  If the lowest element from
        // the first interval is unsigned larger than the largest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedTo.ule(bb.mUnsignedFrom) || &a == &b)
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mUnsignedFrom.uge(bb.mUnsignedTo))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        // If the lowest element from the first interval is
        // signed greater than the largest element from the second
        // interval, the result is 1.  If the largest element from
        // the first interval is signed lower than the lowest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedFrom.sgt(bb.mSignedTo))
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, false))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first interval is
        // signed greater or equal than the largest element from the second
        // interval, the result is 1.  If the largest element from
        // the first interval is signed lower than the lowest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedFrom.sge(bb.mSignedTo) || &a == &b)
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mSignedTo.sle(bb.mSignedFrom))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first interval is
        // signed lower than the lowest element from the second
        // interval, the result is 1.  If the lowest element from
        // the first interval is signed larger than the largest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedTo.slt(bb.mSignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, false))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first interval is
        // signed lower or equal the lowest element from the second
        // interval, the result is 1.  If the lowest element from
        // the first interval is signed larger than the largest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedTo.sle(bb.mSignedFrom) || &a == &b)
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mSignedFrom.sge(bb.mSignedTo))
            setTop();

        break;
    default:
        CANAL_DIE();
    }
}

void
Interval::fcmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{
    const Float::Interval &aa = dynCast<const Float::Interval&>(a),
        &bb = dynCast<const Float::Interval&>(b);

    int result = aa.compare(bb, predicate);
    switch (result)
    {
    case -1:
        setBottom();
        break;
    case 0:
        mEmpty = mSignedTop = mUnsignedTop = false;
        mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo =
            llvm::APInt(/*bitWidth*/1, /*val*/0);
        break;
    case 1:
        mEmpty = mSignedTop = mUnsignedTop = false;
        mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo =
            llvm::APInt(/*bitWidth*/1, /*val*/1);
        break;
    case 2:
        setTop();
        break;
    default:
        CANAL_DIE();
    }
}

void
Interval::trunc(const Domain &value)
{
    const Interval &interval = dynCast<const Interval&>(value);
    mEmpty = interval.mEmpty;
    mSignedTop = interval.mSignedTop;
    mSignedFrom = APIntUtils::trunc(interval.mSignedFrom, getBitWidth());
    mSignedTo = APIntUtils::trunc(interval.mSignedTo, getBitWidth());
    mUnsignedTop = interval.mUnsignedTop;
    mUnsignedFrom = APIntUtils::trunc(interval.mUnsignedFrom, getBitWidth());
    mUnsignedTo = APIntUtils::trunc(interval.mUnsignedTo, getBitWidth());
}

void
Interval::zext(const Domain &value)
{
    const Interval &interval = dynCast<const Interval&>(value);
    mEmpty = interval.mEmpty;
    mSignedTop = interval.mSignedTop;
    mSignedFrom = APIntUtils::zext(interval.mSignedFrom, getBitWidth());
    mSignedTo = APIntUtils::zext(interval.mSignedTo, getBitWidth());
    mUnsignedTop = interval.mUnsignedTop;
    mUnsignedFrom = APIntUtils::zext(interval.mUnsignedFrom, getBitWidth());
    mUnsignedTo = APIntUtils::zext(interval.mUnsignedTo, getBitWidth());
}

void
Interval::sext(const Domain &value)
{
    const Interval &interval = dynCast<const Interval&>(value);
    mEmpty = interval.mEmpty;
    mSignedTop = interval.mSignedTop;
    mSignedFrom = APIntUtils::sext(interval.mSignedFrom, getBitWidth());
    mSignedTo = APIntUtils::sext(interval.mSignedTo, getBitWidth());
    mUnsignedTop = interval.mUnsignedTop;
    mUnsignedFrom = APIntUtils::sext(interval.mUnsignedFrom, getBitWidth());
    mUnsignedTo = APIntUtils::sext(interval.mUnsignedTo, getBitWidth());
}

void
Interval::fptoui(const Domain &value)
{
    setTop();
}

void
Interval::fptosi(const Domain &value)
{
    setTop();
}

float
Interval::accuracy() const
{
    if (mEmpty)
        return 1.0f;

    float coverage = 0;
    if (mUnsignedTop)
        coverage += 1.0f;
    else
    {
        llvm::APInt dividendInt(mUnsignedTo.getBitWidth() + 1, 0);
        dividendInt = mUnsignedTo;
        dividendInt -= mUnsignedFrom;
        dividendInt = dividendInt + 1;

        llvm::APFloat dividendFloat(llvm::APFloat::IEEEdouble);
        llvm::APFloat::opStatus status = dividendFloat.convertFromAPInt(
            dividendInt,
            /*isSigned=*/false,
            llvm::APFloat::rmNearestTiesToEven);
        CANAL_ASSERT(status == llvm::APFloat::opOK);

        llvm::APFloat divisorFloat(llvm::APFloat::IEEEdouble);
        status = dividendFloat.convertFromAPInt(
            llvm::APInt::getMaxValue(mUnsignedTo.getBitWidth() + 1),
            /*isSigned=*/false,
            llvm::APFloat::rmNearestTiesToEven);
        CANAL_ASSERT(status == llvm::APFloat::opOK);

        status = dividendFloat.divide(divisorFloat,
                                      llvm::APFloat::rmNearestTiesToEven);
        CANAL_ASSERT(status == llvm::APFloat::opOK);

        coverage += dividendFloat.convertToFloat();
    }

    if (mSignedTop)
        coverage += 1.0f;
    else
    {
        llvm::APInt dividendInt(mSignedTo.getBitWidth() + 1, 0, /*isSigned=*/true);
        dividendInt = mSignedTo;
        dividendInt -= mSignedFrom;
        dividendInt = dividendInt + 1;

        llvm::APFloat dividendFloat(llvm::APFloat::IEEEdouble);
        llvm::APFloat::opStatus status = dividendFloat.convertFromAPInt(
            dividendInt,
            /*isSigned=*/true,
            llvm::APFloat::rmNearestTiesToEven);
        CANAL_ASSERT(status == llvm::APFloat::opOK);

        llvm::APFloat divisorFloat(llvm::APFloat::IEEEdouble);
        status = dividendFloat.convertFromAPInt(
            llvm::APInt::getMaxValue(mSignedTo.getBitWidth() + 1),
            /*isSigned=*/false,
            llvm::APFloat::rmNearestTiesToEven);
        CANAL_ASSERT(status == llvm::APFloat::opOK);

        status = dividendFloat.divide(divisorFloat,
                                      llvm::APFloat::rmNearestTiesToEven);
        CANAL_ASSERT(status == llvm::APFloat::opOK);

        coverage += dividendFloat.convertToFloat();
    }

    coverage /= 2.0f;
    coverage = 1.0f - coverage;
    return coverage > 1.0f ? 1.0f : (coverage < 0.0f ? 0.0f : coverage);
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
    mUnsignedTop = mSignedTop = false;
    mUnsignedFrom = mUnsignedTo = mSignedFrom = mSignedTo = 0;
}

bool
Interval::isTop() const
{
    return !mEmpty && mSignedTop && mUnsignedTop;
}

void
Interval::setTop()
{
    mEmpty = false;
    mSignedTop = mUnsignedTop = true;
}

} // namespace Integer
} // namespace Canal
