#include "IntegerInterval.h"
#include "Utils.h"
#include "APIntUtils.h"
#include "FloatInterval.h"
#include "Environment.h"

namespace Canal {
namespace Integer {

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
        if (i.sge(j))
        {
            b.signedMax(i);
            a.signedMin(j);
            if (i.sge(j)) {
                return true;
            }
        }
    }

    if (unsigned_)
    {
        a.unsignedMax(i);
        b.unsignedMin(j);
        if (i.uge(j))
        {
            b.unsignedMax(i);
            a.unsignedMin(j);
            if (i.uge(j))
                return true;
        }
    }

    return false;
}


Interval::Interval(const Environment &environment,
                   unsigned bitWidth)
    : Domain(environment, Domain::IntegerIntervalKind),
      mSignedBottom(true),
      mSignedTop(false),
      mSignedFrom(bitWidth, 0, true),
      mSignedTo(bitWidth, 0, true),
      mUnsignedBottom(true),
      mUnsignedTop(false),
      mUnsignedFrom(bitWidth, 0),
      mUnsignedTo(bitWidth, 0)
{
}

Interval::Interval(const Environment &environment,
                   const llvm::APInt &constant)
    : Domain(environment, Domain::IntegerIntervalKind),
      mSignedBottom(false),
      mSignedTop(false),
      mSignedFrom(constant),
      mSignedTo(constant),
      mUnsignedBottom(false),
      mUnsignedTop(false),
      mUnsignedFrom(constant),
      mUnsignedTo(constant)
{
}

Interval::Interval(const Environment &environment,
                   const llvm::APInt &from,
                   const llvm::APInt &to)
    : Domain(environment, Domain::IntegerIntervalKind),
      mSignedBottom(false),
      mSignedTop(false),
      mSignedFrom(from),
      mSignedTo(to),
      mUnsignedBottom(false),
      mUnsignedTop(false),
      mUnsignedFrom(from),
      mUnsignedTo(to)
{
}

Interval::Interval(const Interval &value)
    : Domain(value),
      mSignedBottom(value.mSignedBottom),
      mSignedTop(value.mSignedTop),
      mSignedFrom(value.mSignedFrom),
      mSignedTo(value.mSignedTo),
      mUnsignedBottom(value.mSignedBottom),
      mUnsignedTop(value.mUnsignedTop),
      mUnsignedFrom(value.mUnsignedFrom),
      mUnsignedTo(value.mUnsignedTo)
{
}

bool
Interval::signedMin(llvm::APInt &result) const
{
    if (isSignedBottom())
        return false;

    if (isSignedTop())
        result = llvm::APInt::getSignedMinValue(mSignedFrom.getBitWidth());
    else
        result = mSignedFrom;

    return true;
}

bool
Interval::signedMax(llvm::APInt &result) const
{
    if (isSignedBottom())
        return false;

    if (isSignedTop())
        result = llvm::APInt::getSignedMaxValue(mSignedTo.getBitWidth());
    else
        result = mSignedTo;

    return true;
}

bool
Interval::unsignedMin(llvm::APInt &result) const
{
    if (isUnsignedBottom())
        return false;

    if (isUnsignedTop())
        result = llvm::APInt::getMinValue(mUnsignedFrom.getBitWidth());
    else
        result = mUnsignedFrom;

    return true;
}

bool
Interval::unsignedMax(llvm::APInt &result) const
{
    if (isUnsignedBottom())
        return false;

    if (isUnsignedTop())
        result = llvm::APInt::getMaxValue(mUnsignedTo.getBitWidth());
    else
        result = mUnsignedTo;

    return true;
}

bool
Interval::isConstant() const
{
    return isSignedConstant() && isUnsignedConstant();
}

bool
Interval::isSignedConstant() const
{
    if (isSignedBottom() || isSignedTop())
        return false;

    return mSignedFrom == mSignedTo;
}

bool
Interval::isUnsignedConstant() const
{
    if (isUnsignedBottom() || isUnsignedTop())
        return false;

    return mUnsignedFrom == mUnsignedTo;
}

bool
Interval::isTrue() const
{
    return
        !isBottom() &&
        !isTop() &&
        getBitWidth() == 1 &&
        mSignedFrom.getBoolValue() &&
        mSignedTo.getBoolValue() &&
        mUnsignedFrom.getBoolValue() &&
        mUnsignedTo.getBoolValue();
}

bool
Interval::isFalse() const
{
    return
        !isBottom() &&
        !isTop() &&
        getBitWidth() == 1 &&
        !mSignedFrom.getBoolValue() &&
        !mSignedTo.getBoolValue() &&
        !mUnsignedFrom.getBoolValue() &&
        !mUnsignedTo.getBoolValue();
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
    ss << "interval";
    StringStream sign, unsign;
    if (isBottom()) {
        ss << " empty\n";
        return ss.str();
    }

    if (isSignedBottom())
        sign << " (empty)\n";
    else
    {
        llvm::APInt sMin, sMax;
        bool success = signedMin(sMin);
        CANAL_ASSERT(success);
        success = signedMax(sMax);
        CANAL_ASSERT(success);

        sign << sMin.toString(10, true) << " to "
             << sMax.toString(10, true);

        if (isSignedTop())
            sign << " (top)";
    }

    if (isUnsignedBottom()) {
        unsign << " (empty)\n";
    }
    else
    {
        llvm::APInt uMin, uMax;
        bool success = unsignedMin(uMin);
        CANAL_ASSERT(success);
        success = unsignedMax(uMax);
        CANAL_ASSERT(success);

        unsign << uMin.toString(10, false) << " to "
               << uMax.toString(10, false);

        if (isUnsignedTop())
            unsign << " (top)";
    }
    if (sign.str() != unsign.str())
    {
        ss << "\n"
           << "    signed " << sign.str() << "\n"
           << "    unsigned " << unsign.str() << "\n";
    }
    else
        ss << " " << sign.str() << "\n";

    return ss.str();
}

void
Interval::setZero(const llvm::Value *place)
{
    resetSignedFlags();
    resetUnsignedFlags();
    mUnsignedFrom = mUnsignedTo = mSignedFrom = mSignedTo =
        llvm::APInt::getNullValue(mUnsignedFrom.getBitWidth());
}

bool
Interval::operator==(const Domain& value) const
{
    if (this == &value)
        return true;

    const Interval &interval = checkedCast<Interval>(value);

    if (getBitWidth() != interval.getBitWidth())
        return false;

    if (isSignedBottom() ^ interval.isSignedBottom() ||
        isUnsignedBottom() ^ interval.isUnsignedBottom())
        return false;

    if (isSignedTop() ^ interval.isSignedTop() ||
        isUnsignedTop() ^ interval.isUnsignedTop())
        return false;

    if (!isSignedTop() && !isSignedBottom()
                && (mSignedFrom != interval.mSignedFrom ||
                        mSignedTo != interval.mSignedTo))
    {
        return false;
    }

    if (!isUnsignedTop() && !isUnsignedBottom()
                && (mUnsignedFrom != interval.mUnsignedFrom ||
                          mUnsignedTo != interval.mUnsignedTo))
    {
        return false;
    }

    return true;
}

bool
Interval::operator<(const Domain& value) const
{
    CANAL_NOT_IMPLEMENTED();
}

#define COPY_SIGNED(x) mSignedBottom = x.mSignedBottom; mSignedTop = x.mSignedTop; \
    mSignedFrom = x.mSignedFrom; mSignedTo = x.mSignedTo;
#define COPY_UNSIGNED(x) mUnsignedBottom = x.mUnsignedBottom; mUnsignedTop = x.mUnsignedTop; \
    mUnsignedFrom = x.mUnsignedFrom; mUnsignedTo = x.mUnsignedTo;
Interval &
Interval::join(const Domain &value)
{
    const Interval &interval = checkedCast<Interval>(value);

    CANAL_ASSERT_MSG(interval.getBitWidth() == getBitWidth(),
                     "Bit width must be the same in merge! "
                     << getBitWidth() << " != "
                     << interval.getBitWidth());

    if (isSignedBottom() || interval.isSignedBottom())
    { //Handle bottom values
        //If both are bottom, do nothing (the result is bottom and you are already bottom)
        //If the other is bottom, you don't need to do anything
        //If I am bottom, copy values from the other
        // -> therefore only if I am bottom and the other one is not, copy values from the other one
        if(isSignedBottom() && !interval.isSignedBottom()) {
            COPY_SIGNED(interval);
        }
    }
    else if (!isSignedTop())
    {
        if (interval.isSignedTop())
            setSignedTop();
        else
        {
            if (!mSignedFrom.sle(interval.mSignedFrom))
                mSignedFrom = interval.mSignedFrom;

            if (!mSignedTo.sge(interval.mSignedTo))
                mSignedTo = interval.mSignedTo;
        }
    }

    if (isUnsignedBottom() || interval.isUnsignedBottom())
    { //Handle bottom values
        //If both are bottom, do nothing (the result is bottom and you are already bottom)
        //If the other is bottom, you don't need to do anything
        //If I am bottom, copy values from the other
        // -> therefore only if I am bottom and the other one is not, copy values from the other one
        if(isUnsignedBottom() && !interval.isUnsignedBottom()) {
            COPY_UNSIGNED(interval);
        }
    }
    else if (!isUnsignedTop())
    {
        if (interval.isUnsignedTop())
            setUnsignedTop();
        else
        {
            if (!mUnsignedFrom.ule(interval.mUnsignedFrom))
                mUnsignedFrom = interval.mUnsignedFrom;

            if (!mUnsignedTo.uge(interval.mUnsignedTo))
                mUnsignedTo = interval.mUnsignedTo;
        }
    }

    return *this;
}

Interval &
Interval::meet(const Domain &value)
{
    const Interval &interval = checkedCast<Interval>(value);

    if (*this == interval) return *this; //Shortcut

    if (isSignedBottom() || interval.isSignedBottom()) {
        setSignedBottom();
    }
    else if (isSignedTop() || interval.isSignedTop()) {
        //If both are top, do nothing (the result is top and you are already top)
        //If the other is top, you don't need to do anything
        //If I am top, copy values from the other
        // -> therefore only if I am top and the other one is not, copy values from the other one
        if (isSignedTop() && !interval.isSignedTop()) {
            COPY_SIGNED(interval);
        }
    }
    else {
        //If there is no intersection, set to bottom; otherwise use the intersection
        if (intersects(*this, interval, true, false)) {
            //If there is intersection, intervals are like this:
            // < < > > - but you do not know, which one is outer and which boundary is which
            //Therefore intersection is interval from bigger minimum to lower maximum
            if (mSignedFrom.slt(interval.mSignedFrom)) mSignedFrom = interval.mSignedFrom;
            if (mSignedTo.sgt(interval.mSignedTo)) mSignedTo = interval.mSignedTo;
        }
        else {
            setSignedBottom();
        }
    }

    if (isUnsignedBottom() || interval.isUnsignedBottom()) {
        setUnsignedBottom();
    }
    else if (isUnsignedTop() || interval.isUnsignedTop()) {
        if (isUnsignedTop() && !interval.isUnsignedTop()) {
            COPY_UNSIGNED(interval);
        }
    }
    else {
        if (intersects(*this, interval, false, true)) {
            if (mUnsignedFrom.ult(interval.mUnsignedFrom)) mUnsignedFrom = interval.mUnsignedFrom;
            if (mUnsignedTo.ugt(interval.mUnsignedTo)) mUnsignedTo = interval.mUnsignedTo;
        }
        else {
            setUnsignedBottom();
        }
    }
    return *this;
}
#undef COPY_UNSIGNED
#undef COPY_SIGNED

void
Interval::setSignedBottom() {
    mSignedBottom = true;
    mSignedTop = false;
}

void
Interval::setSignedTop() {
    mSignedTop = true;
    mSignedBottom = false;
}

bool
Interval::isSignedBottom() const {
    return mSignedBottom && !mSignedTop;
}

bool
Interval::isSignedTop() const {
    return !mSignedBottom && mSignedTop;
}

void
Interval::resetSignedFlags() {
    mSignedTop = mSignedBottom = false;
}

void
Interval::setUnsignedBottom() {
    mUnsignedBottom = true;
    mUnsignedTop = false;
}

void
Interval::setUnsignedTop() {
    mUnsignedTop = true;
    mUnsignedBottom = false;
}

bool
Interval::isUnsignedBottom() const {
    return mUnsignedBottom && !mUnsignedTop;
}

bool
Interval::isUnsignedTop() const {
    return !mUnsignedBottom && mUnsignedTop;
}

void
Interval::resetUnsignedFlags() {
    mUnsignedTop = mUnsignedBottom = false;
}

void
Interval::resetFlags() {
    resetSignedFlags();
    resetUnsignedFlags();
}

bool
Interval::isBottom() const
{
    return isSignedBottom() && isUnsignedBottom();
}

void
Interval::setBottom()
{
    setSignedBottom();
    setUnsignedBottom();
}

bool
Interval::isTop() const
{
    return isSignedTop() && isUnsignedTop();
}

void
Interval::setTop()
{
    setSignedTop();
    setUnsignedTop();
}

float
Interval::accuracy() const
{
    float coverage = 0;
    if (isUnsignedTop() || isUnsignedBottom())
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

    if (isSignedTop() || isSignedBottom())
        coverage += 1.0f;
    else
    {
        llvm::APInt dividendInt(mSignedTo.getBitWidth() + 1,
                                0,
                                /*isSigned=*/true);

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

Interval &
Interval::add(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    if (aa.isSignedBottom() || bb.isSignedBottom()) {
        setSignedBottom();
    }
    else if (aa.isSignedTop() || bb.isSignedTop()) {
        setSignedTop();
    }
    else {
        resetSignedFlags();
        mSignedFrom = APIntUtils::sadd_ov(aa.mSignedFrom,
                                          bb.mSignedFrom,
                                          mSignedTop);

        if (!mSignedTop)
            mSignedTo = APIntUtils::sadd_ov(aa.mSignedTo,
                                            bb.mSignedTo,
                                            mSignedTop);
    }

    if (aa.isUnsignedBottom() || bb.isUnsignedBottom()) {
        setUnsignedBottom();
    }
    else if (aa.isUnsignedTop() || bb.isUnsignedTop()) {
        setUnsignedTop();
    }
    else {
        resetUnsignedFlags();
        mUnsignedFrom = APIntUtils::uadd_ov(aa.mUnsignedFrom,
                                            bb.mUnsignedFrom,
                                            mUnsignedTop);

        if (!mUnsignedTop)
            mUnsignedTo = APIntUtils::uadd_ov(aa.mUnsignedTo,
                                              bb.mUnsignedTo,
                                              mUnsignedTop);
    }

    return *this;
}

Interval &
Interval::sub(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    if (aa.isSignedBottom() || bb.isSignedBottom()) {
        setSignedBottom();
    }
    else if (aa.isSignedTop() || bb.isSignedTop()) {
        setSignedTop();
    }
    else {
        resetSignedFlags();
        mSignedFrom = APIntUtils::ssub_ov(aa.mSignedFrom,
                                          bb.mSignedTo,
                                          mSignedTop);

        if (!mSignedTop)
            mSignedTo = APIntUtils::ssub_ov(aa.mSignedTo,
                                            bb.mSignedFrom,
                                            mSignedTop);
    }

    if (aa.isUnsignedBottom() || bb.isUnsignedBottom()) {
        setUnsignedBottom();
    }
    else if (aa.isUnsignedTop() || bb.isUnsignedTop()) {
        setUnsignedTop();
    }
    else {
        resetUnsignedFlags();
        mUnsignedFrom = APIntUtils::usub_ov(aa.mUnsignedFrom,
                                            bb.mUnsignedTo,
                                            mUnsignedTop);

        if (!mUnsignedTop)
            mUnsignedTo = APIntUtils::usub_ov(aa.mUnsignedTo,
                                              bb.mUnsignedFrom,
                                              mUnsignedTop);
    }

    return *this;
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
    LessThan lt(isSigned
                ? (LessThan)&llvm::APInt::slt
                : (LessThan)&llvm::APInt::ult);

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

Interval &
Interval::mul(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    if (aa.isSignedBottom() || bb.isSignedBottom()) {
        setSignedBottom();
    }
    else if (aa.isSignedTop() || bb.isSignedTop()) {
        setSignedTop();
    }
    else {
        resetSignedFlags();
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

    if (aa.isUnsignedBottom() || bb.isUnsignedBottom()) {
        setUnsignedBottom();
    }
    else if (aa.isUnsignedTop() || bb.isUnsignedTop()) {
        setUnsignedTop();
    }
    else {
        resetUnsignedFlags();
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

    return *this;
}

Interval &
Interval::udiv(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setSignedTop();

    if (aa.isUnsignedBottom() || bb.isUnsignedBottom()) {
        setUnsignedBottom();
    }
    else if (aa.isUnsignedTop() || bb.isUnsignedTop()) {
        setUnsignedTop();
    }
    else {
        resetUnsignedFlags();

        if (bb.mUnsignedFrom == 0 && bb.mUnsignedTo == 0) { //Division by zero
            setUnsignedTop();
            return *this;
        }
        llvm::APInt toTo, toFrom, fromTo, fromFrom;
        if (bb.mUnsignedFrom == 0) { //From is zero -> divide by 1 -> same as original
            toFrom = aa.mUnsignedTo;
            fromFrom = aa.mUnsignedFrom;
        }
        else {
            toFrom = aa.mUnsignedTo.udiv(bb.mUnsignedFrom);
            fromFrom = aa.mUnsignedFrom.udiv(bb.mUnsignedFrom);
        }
        //mUnsignedTo cannot be zero, because this is unsigned division and from <= to (from = to is already handled)
        toTo = aa.mUnsignedTo.udiv(bb.mUnsignedTo);
        fromTo = aa.mUnsignedFrom.udiv(bb.mUnsignedTo);

        minMax(/*signed=*/false,
               mUnsignedFrom,
               mUnsignedTo,
               toTo,
               toFrom,
               fromTo,
               fromFrom);
    }

    return *this;
}

Interval &
Interval::sdiv(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setUnsignedTop();

    if (aa.isSignedBottom() || bb.isSignedBottom()) {
        setSignedBottom();
    }
    else if (aa.isSignedTop() || bb.isSignedTop()) {
        setSignedTop();
    }
    else {
        resetSignedFlags();
        if (bb.mSignedFrom == 0 && bb.mSignedTo == 0) { //Division by zero
            setSignedTop();
            return *this;
        }
        llvm::APInt toTo, toFrom, fromTo, fromFrom;
        if (bb.mSignedFrom == 0) { //From is zero -> divide by 1 -> same as original
            fromFrom = aa.mSignedFrom;
            toFrom = aa.mSignedTo;
        }
        else {
            fromFrom = APIntUtils::sdiv_ov(aa.mSignedFrom, bb.mSignedFrom, mSignedTop);
            if (!mSignedTop) {
                toFrom = APIntUtils::sdiv_ov(aa.mSignedTo, bb.mSignedFrom, mSignedTop);
            }
        }
        if (!mSignedTop) {
            if (bb.mSignedTo == 0) { //To is zero -> divide by -1
                fromTo = APIntUtils::sdiv_ov(aa.mSignedFrom, llvm::APInt(bb.getBitWidth(), -1, true), mSignedTop);
                if (!mSignedTop) {
                    toTo = APIntUtils::sdiv_ov(aa.mSignedTo, llvm::APInt(bb.getBitWidth(), -1, true), mSignedTop);
                }
            }
            else {
                fromTo = APIntUtils::sdiv_ov(aa.mSignedFrom, bb.mSignedTo, mSignedTop);
                if (!mSignedTop) {
                    toTo = APIntUtils::sdiv_ov(aa.mSignedTo, bb.mSignedTo, mSignedTop);
                }
            }
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

    return *this;
}

Interval &
Interval::urem(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    //When implemented, handle division by zero - see #145
    return *this;
}

Interval &
Interval::srem(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    //When implemented, handle division by zero - see #145
    return *this;
}

Interval &
Interval::shl(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    return *this;
}

Interval &
Interval::lshr(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    return *this;
}

Interval &
Interval::ashr(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    return *this;
}

Interval &
Interval::and_(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    return *this;
}

Interval &
Interval::or_(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    return *this;
}

Interval &
Interval::xor_(const Domain &a, const Domain &b)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    setTop();

    return *this;
}

Interval &
Interval::icmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{
    const Interval &aa = checkedCast<Interval>(a),
        &bb = checkedCast<Interval>(b);

    if (aa.isTop() || bb.isTop())
    {
        // Result could be both true and false.
        setTop();
        return *this;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        // Result is undefined.
        setBottom();
        return *this;
    }

    setZero(NULL); // Interval 0-0

    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If both intervals are an equal constant, the result is 1.
        // If interval intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).

        // Signed equality
        if (aa.isSignedConstant() &&
            bb.isSignedConstant() &&
            aa.mSignedFrom == bb.mSignedFrom)
        {
            mSignedFrom = mSignedTo = 1;
        }
        else if (intersects(aa, bb, true, false))
            setSignedTop();

        // Unsigned equality
        if (aa.isUnsignedConstant() &&
            bb.isUnsignedConstant() &&
            aa.mUnsignedFrom == bb.mUnsignedFrom)
        {
            mUnsignedFrom = mUnsignedTo = 1;
        }
        else if (intersects(aa, bb, false, true))
            setUnsignedTop();

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If interval intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).

        // Signed inequality.
        if (!(aa.isSignedConstant() &&
              bb.isSignedConstant() &&
              aa.mSignedFrom == bb.mSignedFrom))
        {
            if (intersects(aa, bb, true, false))
                setSignedTop();
            else
                mSignedFrom = mSignedTo = 1;
        }

        // Unsigned inequality.
        if (!(aa.isUnsignedConstant() &&
              bb.isUnsignedConstant() &&
              aa.mUnsignedFrom == bb.mUnsignedFrom))
        {
            if (intersects(aa, bb, false, true))
                setUnsignedTop();
            else
                mUnsignedFrom = mUnsignedTo = 1;
        }

        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        if (aa.mUnsignedFrom.ugt(bb.mUnsignedTo))
        {
            // If the lowest element from the first interval is
            // unsigned greater than the largest element from the
            // second interval, the result is 1.
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        }
        else if (aa.mUnsignedTo != bb.mUnsignedFrom &&
                 intersects(aa, bb, false, true))
        {
            // If the largest element from the first interval is
            // unsigned lower than or equal to the lowest element from
            // the second interval, the result is 0.  Otherwise the
            // result is the top value (both 0 and 1).
            setTop();
        }

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the lowest element from the first interval is
        // unsigned greater or equal than the largest element from the second
        // interval, the result is 1.  If the largest element from
        // the first interval is unsigned lower than the lowest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedFrom.uge(bb.mUnsignedTo))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mUnsignedTo.ult(bb.mUnsignedFrom))
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first interval is unsigned
        // lower than the lowest element from the second interval, the
        // result is 1.  If the lowest element from the first interval
        // is unsigned larger than or equal to the largest element
        // from the second interval, the result is 0.  Otherwise the
        // result is the top value (both 0 and 1).
        if (aa.mUnsignedTo.ult(bb.mUnsignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (aa.mUnsignedFrom != bb.mUnsignedTo &&
                 intersects(aa, bb, false, true))
        {
            setTop();
        }

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first interval is
        // unsigned lower or equal the lowest element from the second
        // interval, the result is 1.  If the lowest element from
        // the first interval is unsigned larger than the largest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mUnsignedTo.ule(bb.mUnsignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mUnsignedFrom.ugt(bb.mUnsignedTo))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        if (aa.mSignedFrom.sgt(bb.mSignedTo))
        {
            // If the lowest element from the first interval is signed
            // greater than the largest element from the second
            // interval, the first interval is greater than the
            // second.
            mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        }
        else if (aa.mSignedTo != bb.mSignedFrom &&
                 intersects(aa, bb, true, false))
        {
            // Otherwise the result is the top value (both 0 and 1).
            setTop();
        }

        // If the largest element from the first interval is
        // signed lower of equal to the lowest element from the
        // second interval, the first interval can never be lower
        // than the second.
        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first interval is
        // signed greater or equal than the largest element from the second
        // interval, the result is 1.  If the largest element from
        // the first interval is signed lower than the lowest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedFrom.sge(bb.mSignedTo))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mSignedTo.slt(bb.mSignedFrom))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first interval is signed
        // lower than the lowest element from the second interval, the
        // result is 1.  If the lowest element from the first interval
        // is signed larger than or equal to the largest element from
        // the second interval, the result is 0.  Otherwise the result
        // is the top value (both 0 and 1).
        if (aa.mSignedTo.slt(bb.mSignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (aa.mSignedFrom != bb.mSignedTo &&
                 intersects(aa, bb, true, false))
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first interval is
        // signed lower or equal the lowest element from the second
        // interval, the result is 1.  If the lowest element from
        // the first interval is signed larger than the largest
        // element from the second interval, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mSignedTo.sle(bb.mSignedFrom))
           mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo = 1;
        else if (!aa.mSignedFrom.sgt(bb.mSignedTo))
            setTop();

        break;
    default:
        CANAL_DIE();
    }

    return *this;
}

Interval &
Interval::fcmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{
    const Float::Interval &aa = checkedCast<Float::Interval>(a),
        &bb = checkedCast<Float::Interval>(b);

    int result = aa.compare(bb, predicate);
    switch (result)
    {
    case -1:
        setBottom();
        break;
    case 0:
        resetFlags();
        mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo =
            llvm::APInt(/*bitWidth*/1, /*val*/0);

        break;
    case 1:
        resetFlags();
        mSignedFrom = mSignedTo = mUnsignedFrom = mUnsignedTo =
            llvm::APInt(/*bitWidth*/1, /*val*/1);

        break;
    case 2:
        setTop();
        break;
    default:
        CANAL_DIE();
    }

    return *this;
}

Interval &
Interval::trunc(const Domain &value)
{
    const Interval &interval = checkedCast<Interval>(value);

    if (!interval.isSignedBottom()) {
        resetSignedFlags();
        mSignedTop = interval.mSignedTop
            || (!interval.mSignedFrom.isSignedIntN(getBitWidth()) &&
                !interval.mSignedFrom.isIntN(getBitWidth()))
            || (!interval.mSignedTo.isSignedIntN(getBitWidth()) &&
                !interval.mSignedTo.isIntN(getBitWidth()));

        if (!mSignedTop)
        {
            mSignedFrom = APIntUtils::trunc(interval.mSignedFrom,
                                            getBitWidth());

            mSignedTo = APIntUtils::trunc(interval.mSignedTo,
                                          getBitWidth());

            if (getBitWidth() == 1 && mSignedFrom != mSignedTo)
                setSignedTop();
            else
            {
                if (!mSignedFrom.isNegative() && mSignedTo.isNegative()) {
                    //If during truncate mTo became negative and mFrom stayed positive,
                    //mFrom and mTo have to be swapped
                    std::swap(mSignedFrom, mSignedTo);
                }
                CANAL_ASSERT_MSG(mSignedFrom.sle(mSignedTo),
                                 "mSignedFrom must be lower than mSignedTo");
            }
        }
    }

    if (!interval.isUnsignedBottom()) {
        mUnsignedBottom = false;
        mUnsignedTop = interval.mUnsignedTop
            || !interval.mUnsignedFrom.isIntN(getBitWidth())
            || !interval.mUnsignedTo.isIntN(getBitWidth());

        if (!mUnsignedTop)
        {
            mUnsignedFrom = APIntUtils::trunc(interval.mUnsignedFrom,
                                              getBitWidth());

            mUnsignedTo = APIntUtils::trunc(interval.mUnsignedTo,
                                            getBitWidth());

            CANAL_ASSERT_MSG(mUnsignedFrom.ule(mUnsignedTo),
                             "mUnsignedFrom must be lower than mUnsignedTo");
        }
    }
    return *this;
}

Interval &
Interval::zext(const Domain &value)
{
    const Interval &interval = checkedCast<Interval>(value);
    mSignedBottom = interval.mSignedBottom;
    mSignedTop = interval.mSignedTop;
    if (!mSignedBottom && !mSignedTop) {
        mSignedFrom = APIntUtils::zext(interval.mSignedFrom, getBitWidth());
        mSignedTo = APIntUtils::zext(interval.mSignedTo, getBitWidth());
    }

    mUnsignedBottom = interval.mUnsignedBottom;
    mUnsignedTop = interval.mUnsignedTop;
    if (!mUnsignedBottom && !mUnsignedTop) {
        mUnsignedFrom = APIntUtils::zext(interval.mUnsignedFrom, getBitWidth());
        mUnsignedTo = APIntUtils::zext(interval.mUnsignedTo, getBitWidth());
    }
    return *this;
}

Interval &
Interval::sext(const Domain &value)
{
    const Interval &interval = checkedCast<Interval>(value);
    mSignedBottom = interval.mSignedBottom;
    mSignedTop = interval.mSignedTop;
    if (!mSignedBottom && !mSignedTop) {
        mSignedFrom = APIntUtils::sext(interval.mSignedFrom, getBitWidth());
        mSignedTo = APIntUtils::sext(interval.mSignedTo, getBitWidth());
    }

    mUnsignedBottom = interval.mUnsignedBottom;
    mUnsignedTop = interval.mUnsignedTop;
    if (!mUnsignedBottom && !mUnsignedTop) {
        mUnsignedFrom = APIntUtils::sext(interval.mUnsignedFrom, getBitWidth());
        mUnsignedTo = APIntUtils::sext(interval.mUnsignedTo, getBitWidth());
    }
    return *this;
}

Interval &
Interval::fptoui(const Domain &value)
{
    setTop();
    return *this;
}

Interval &
Interval::fptosi(const Domain &value)
{
    setTop();
    return *this;
}

const llvm::IntegerType &
Interval::getValueType() const
{
    return *llvm::Type::getIntNTy(mEnvironment.getContext(), getBitWidth());
}

} // namespace Integer
} // namespace Canal
