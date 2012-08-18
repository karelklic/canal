#include "IntegerRange.h"
#include "Constant.h"
#include "Utils.h"
#include "APIntUtils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Range::Range(unsigned numBits)
    : mEmpty(true),
      mSignedTop(false),
      mSignedFrom(numBits, 0),
      mSignedTo(numBits, 0),
      mUnsignedTop(false),
      mUnsignedFrom(numBits, 0),
      mUnsignedTo(numBits, 0)
{
}

Range::Range(const llvm::APInt &constant)
    : mEmpty(false),
      mSignedTop(false),
      mSignedFrom(constant),
      mSignedTo(constant),
      mUnsignedTop(false),
      mUnsignedFrom(constant),
      mUnsignedTo(constant)
{
}

bool
Range::signedMin(llvm::APInt &result) const
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
Range::signedMax(llvm::APInt &result) const
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
Range::unsignedMin(llvm::APInt &result) const
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
Range::unsignedMax(llvm::APInt &result) const
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
Range::isSingleValue() const
{
    if (isBottom() || mSignedTop || mUnsignedTop)
        return false;

    return mSignedFrom == mSignedTo &&
        mUnsignedFrom == mUnsignedTo;
}

Range *
Range::clone() const
{
    return new Range(*this);
}

Range *
Range::cloneCleaned() const
{
    return new Range(getBitWidth());
}

bool
Range::operator==(const Value& value) const
{
    const Range *range = dynCast<const Range*>(&value);
    if (!range)
        return false;
    if (mEmpty || range->mEmpty)
        return mEmpty == range->mEmpty;
    if (mSignedTop ^ range->mSignedTop || mUnsignedTop ^ range->mUnsignedTop)
        return false;

    if (!mSignedTop && (mSignedFrom != range->mSignedFrom ||
                        mSignedTo != range->mSignedTo))
    {
        return false;
    }

    if (!mUnsignedTop && (mUnsignedFrom != range->mUnsignedFrom ||
                          mUnsignedTo != range->mUnsignedTo))
    {
        return false;
    }

    return true;
}

void
Range::merge(const Value &value)
{
    // Handle values represeting a constant.
    if (const Constant *constant = dynCast<const Constant*>(&value))
    {
        CANAL_ASSERT(constant->isAPInt());
        const llvm::APInt &constInt = constant->getAPInt();

        if (mEmpty)
        {
            mEmpty = false;
            mSignedFrom = mSignedTo = constInt;
            mUnsignedFrom = mUnsignedTo = constInt;
            return;
        }

        if (!mSignedTop)
        {
            if (!mSignedFrom.sle(constInt))
                mSignedFrom = constInt;
            if (!mSignedTo.sge(constInt))
                mSignedTo = constInt;
        }

        if (!mUnsignedTop)
        {
            if (!mUnsignedFrom.sle(constInt))
                mUnsignedFrom = constInt;
            if (!mUnsignedTo.sge(constInt))
                mUnsignedTo = constInt;
        }

        return;
    }

    const Range &range = dynCast<const Range&>(value);
    if (range.mEmpty)
        return;

    mEmpty = false;

    if (!mSignedTop)
    {
        if (range.mSignedTop)
            mSignedTop = true;
        else
        {
            if (!mSignedFrom.sle(range.mSignedFrom))
                mSignedFrom = range.mSignedFrom;
            if (!mSignedTo.sge(range.mSignedTo))
                mSignedTo = range.mSignedTo;
        }
    }

    if (!mUnsignedTop)
    {
        if (range.mUnsignedTop)
            mUnsignedTop = true;
        else
        {
            if (!mUnsignedFrom.ule(range.mUnsignedFrom))
                mUnsignedFrom = range.mUnsignedFrom;
            if (!mUnsignedTo.uge(range.mUnsignedTo))
                mUnsignedTo = range.mUnsignedTo;
        }
    }
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
    ss << "range";
    if (mEmpty)
        ss << " empty" << std::endl;
    else
    {
        std::stringstream sign;
        sign << mSignedFrom.toString(10, true) << " to "
             << mSignedTo.toString(10, true);
        std::stringstream unsign;
        unsign << mUnsignedFrom.toString(10, false) << " to "
               << mUnsignedTo.toString(10, false);

        if (mSignedTop || mUnsignedTop || sign.str() != unsign.str())
        {
            ss << std::endl;
            ss << "    signed ";
            ss << (mSignedTop ? "-infinity to infinity" : sign.str());
            ss << std::endl;

            ss << "    unsigned ";
            ss << (mUnsignedTop ? "0 to infinity" : unsign.str());
            ss << std::endl;
        }
        else
            ss << " " << sign.str() << std::endl;
    }

    return ss.str();
}

bool
Range::matchesString(const std::string &text,
                     std::string &rationale) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Range::add(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

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
Range::sub(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

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
Range::mul(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

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
Range::udiv(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

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
Range::sdiv(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

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
Range::urem(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::srem(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::shl(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::lshr(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::ashr(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::and_(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::or_(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::xor_(const Value &a, const Value &b)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

// Assumes that neither a nor b are empty or top
static bool
intersects(const Range &a,
           const Range &b,
           bool signed_,
           bool unsigned_)
{
    llvm::APInt i, j;
    if (signed_)
    {
        a.signedMax(i);
        b.signedMin(j);
        if (i.sge(j))
            return true;

        a.signedMin(i);
        b.signedMax(j);
        if (i.sle(j))
            return true;
    }

    if (unsigned_)
    {
        a.unsignedMax(i);
        b.unsignedMin(j);
        if (i.uge(j))
            return true;

        a.unsignedMin(i);
        b.unsignedMax(j);
        if (i.ule(j))
            return true;
    }

    return false;
}

void
Range::icmp(const Value &a, const Value &b,
            llvm::CmpInst::Predicate predicate)
{
    const Range &aa = dynCast<const Range&>(a),
        &bb = dynCast<const Range&>(b);

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

    mEmpty = false;

    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If both ranges are equal, the result is 1.  If
        // range intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b || (aa.isSingleValue() && aa == bb))
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, true))
            setTop();

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If both ranges are equal, the result is 0.  If
        // range intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).
        if (!intersects(aa, bb, true, true))
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!(&a == &b || (aa.isSingleValue() && aa == bb)))
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        // If the lowest element from the first range is
        // unsigned greater than the largest element from the second
        // range, the result is 1.  If the largest element from
        // the first range is unsigned lower than the lowest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).

        if (aa.mUnsignedFrom.ugt(bb.mUnsignedTo))
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, false, true))
            this->setTop();

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the lowest element from the first range is
        // unsigned greater or equal than the largest element from the second
        // range, the result is 1.  If the largest element from
        // the first range is unsigned lower than the lowest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedFrom.uge(bb.mUnsignedTo))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, false, true))
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first range is
        // unsigned lower than the lowest element from the second
        // range, the result is 1.  If the lowest element from
        // the first range is unsigned larger than the largest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedTo.ult(bb.mUnsignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, false, true))
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first range is
        // unsigned lower or equal the lowest element from the second
        // range, the result is 1.  If the lowest element from
        // the first range is unsigned larger than the largest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedTo.ule(bb.mUnsignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, false, true))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        // If the lowest element from the first range is
        // signed greater than the largest element from the second
        // range, the result is 1.  If the largest element from
        // the first range is signed lower than the lowest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedFrom.sgt(bb.mSignedTo))
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, false))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first range is
        // signed greater or equal than the largest element from the second
        // range, the result is 1.  If the largest element from
        // the first range is signed lower than the lowest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedFrom.sge(bb.mSignedTo))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, false))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first range is
        // signed lower than the lowest element from the second
        // range, the result is 1.  If the lowest element from
        // the first range is signed larger than the largest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedTo.slt(bb.mSignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, false))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first range is
        // signed lower or equal the lowest element from the second
        // range, the result is 1.  If the lowest element from
        // the first range is signed larger than the largest
        // element from the second range, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedTo.sle(bb.mSignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (intersects(aa, bb, true, false))
            setTop();

        break;
    default:
        CANAL_DIE();
    }
}

float
Range::accuracy() const
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
    return !mEmpty && mSignedTop && mUnsignedTop;
}

void
Range::setTop()
{
    mEmpty = false;
    mSignedTop = mUnsignedTop = true;
}

} // namespace Integer
} // namespace Canal
