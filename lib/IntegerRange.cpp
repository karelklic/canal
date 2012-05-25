#include "IntegerRange.h"
#include "Constant.h"
#include "Utils.h"
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
    const Range &range = dynamic_cast<const Range&>(value);
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
Range::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Integer::Range: ";
    if (mEmpty)
        ss << "empty";
    else
    {
        ss << "{" << std::endl;
        ss << "  signed: ";
        if (mSignedTop)
            ss << "-infinity to infinity";
        else
        {
            ss << mSignedFrom.toString(10, true) << " to "
               << mSignedTo.toString(10, true);
        }
        ss << std::endl;

        ss << "  unsigned: ";
        if (mUnsignedTop)
            ss << "0 to infinity";
        else
        {
            ss << mUnsignedFrom.toString(10, false) << " to "
               << mUnsignedTo.toString(10, false);
        }
        ss << std::endl;

        ss << "}";
    }
    return ss.str();
}

void
Range::add(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        mSignedFrom = aa.mSignedFrom.sadd_ov(bb.mSignedFrom, mSignedTop);
        if (!mSignedTop)
            mSignedTo = aa.mSignedTo.sadd_ov(bb.mSignedTo, mSignedTop);
    }

    mUnsignedTop = aa.mUnsignedTop || bb.mUnsignedTop;
    if (!mUnsignedTop)
    {
        mUnsignedFrom = aa.mUnsignedFrom.uadd_ov(bb.mUnsignedFrom, mUnsignedTop);
        if (!mUnsignedTop)
            mUnsignedTo = aa.mUnsignedTo.uadd_ov(bb.mUnsignedTo, mUnsignedTop);
    }
}

void
Range::sub(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        mSignedFrom = aa.mSignedFrom.ssub_ov(bb.mSignedTo, mSignedTop);
        if (!mSignedTop)
            mSignedTo = aa.mSignedTo.ssub_ov(bb.mSignedFrom, mSignedTop);
    }

    mUnsignedTop = aa.mUnsignedTop || bb.mUnsignedTop;
    if (!mUnsignedTop)
    {
        mUnsignedFrom = aa.mUnsignedFrom.usub_ov(bb.mUnsignedTo, mUnsignedTop);
        if (!mUnsignedTop)
            mUnsignedTo = aa.mUnsignedTo.usub_ov(bb.mUnsignedFrom, mUnsignedTop);
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
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        llvm::APInt toTo = aa.mSignedTo.smul_ov(bb.mSignedTo,
                                                mSignedTop);
        if (!mSignedTop)
        {
            llvm::APInt toFrom = aa.mSignedTo.smul_ov(bb.mSignedFrom,
                                                      mSignedTop);
            if (!mSignedTop)
            {
                llvm::APInt fromTo = aa.mSignedFrom.smul_ov(bb.mSignedTo,
                                                            mSignedTop);
                if (!mSignedTop)
                {
                    llvm::APInt fromFrom = aa.mSignedFrom.smul_ov(bb.mSignedFrom,
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
        llvm::APInt toTo = aa.mUnsignedTo.umul_ov(bb.mUnsignedTo,
                                                  mUnsignedTop);
        if (!mUnsignedTop)
        {
            llvm::APInt toFrom = aa.mUnsignedTo.umul_ov(bb.mUnsignedFrom,
                                                        mUnsignedTop);
            if (!mUnsignedTop)
            {
                llvm::APInt fromTo = aa.mUnsignedFrom.umul_ov(bb.mUnsignedTo,
                                                              mUnsignedTop);
                if (!mUnsignedTop)
                {
                    llvm::APInt fromFrom = aa.mUnsignedFrom.umul_ov(bb.mUnsignedFrom,
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
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

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
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    mUnsignedTop = true;
    mSignedTop = aa.mSignedTop || bb.mSignedTop;
    if (!mSignedTop)
    {
        llvm::APInt toTo = aa.mSignedTo.sdiv_ov(bb.mSignedTo,
                                                mSignedTop);
        if (!mSignedTop)
        {
            llvm::APInt toFrom = aa.mSignedTo.sdiv_ov(bb.mSignedFrom,
                                                      mSignedTop);
            if (!mSignedTop)
            {
                llvm::APInt fromTo = aa.mSignedFrom.sdiv_ov(bb.mSignedTo,
                                                            mSignedTop);
                if (!mSignedTop)
                {
                    llvm::APInt fromFrom = aa.mSignedFrom.sdiv_ov(bb.mSignedFrom,
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
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::srem(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::shl(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::lshr(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::ashr(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::and_(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::or_(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
}

void
Range::xor_(const Value &a, const Value &b)
{
    const Range &aa = dynamic_cast<const Range&>(a),
        &bb = dynamic_cast<const Range&>(b);

    // Handle empty values.
    mEmpty = (aa.mEmpty || bb.mEmpty);
    if (mEmpty)
        return;

    setTop();
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
