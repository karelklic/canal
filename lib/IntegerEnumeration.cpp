#include "IntegerEnumeration.h"
#include "Utils.h"
#include "FloatInterval.h"
#include "StringUtils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Enumeration::Enumeration(const Environment &environment,
                         unsigned bitWidth)
    : Domain(environment), mTop(false), mBitWidth(bitWidth)
{
    CANAL_ASSERT(mBitWidth > 0);
}

Enumeration::Enumeration(const Environment &environment,
                         const llvm::APInt &number)
    : Domain(environment), mTop(false), mBitWidth(number.getBitWidth())
{
    mValues.insert(number);
    CANAL_ASSERT(mBitWidth > 0);
}

Enumeration::Enumeration(const Enumeration &value)
    : Domain(value),
      mValues(value.mValues),
      mTop(value.mTop),
      mBitWidth(value.mBitWidth)
{
}

bool
Enumeration::signedMin(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getSignedMinValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        //Find lowest negative number
        APIntUtils::USet::const_iterator bound = mValues.lower_bound(llvm::APInt::getSignedMinValue(mBitWidth));
        if (bound == mValues.end())
        {
            // If there is no negative number in this enumeration then
            // the first element in this enumeration is lowest
            result = *mValues.begin();
        }
        else
            result = *bound;
    }

    return true;
}

bool
Enumeration::signedMax(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getSignedMaxValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        //Find lowest negative number
        APIntUtils::USet::const_iterator bound = mValues.lower_bound(llvm::APInt::getSignedMinValue(mBitWidth));
        if (bound == mValues.end() || //If there is no negative number in this enumeration
                bound == mValues.begin()) { //or first element in this enumeration is negative
            result = *mValues.rbegin(); //then the last element in this enumeration is highest
        }
        else { //There are some positive numbers as well
            result = *(--bound); //then the highest number is the one directly preceeding lowest negative number
        }
    }

    return true;
}

bool
Enumeration::unsignedMin(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getMinValue(mBitWidth);
    else {
        if (mValues.empty()) return false;
        // We assume the set is sorted by unsigned comparison.
        result = *mValues.begin();
    }
    return true;
}

bool
Enumeration::unsignedMax(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getMaxValue(mBitWidth);
    else {
        if (mValues.empty()) return false;
        // We assume the set is sorted by unsigned comparison.
        result = *mValues.rbegin();
    }
    return true;
}

bool
Enumeration::isSingleValue() const
{
    return (!mTop && mValues.size() == 1);
}

Enumeration *
Enumeration::clone() const
{
    return new Enumeration(*this);
}

Enumeration *
Enumeration::cloneCleaned() const
{
    return new Enumeration(mEnvironment, getBitWidth());
}

bool
Enumeration::operator==(const Domain &value) const
{
    const Enumeration *enumeration =
        dynCast<const Enumeration*>(&value);

    if (!enumeration)
        return false;
    if (mTop != enumeration->mTop)
        return false;
    if (mTop)
        return true;
    // Compare values only if the top is not set, otherwise we would
    // get false inequality.
    return mValues == enumeration->mValues;
}

void
Enumeration::merge(const Domain &value)
{
    if (mTop)
        return;

    const Enumeration &enumeration =
        dynCast<const Enumeration&>(value);

    if (enumeration.isTop())
        setTop();
    else
    {
        CANAL_ASSERT_MSG(enumeration.getBitWidth() == getBitWidth(),
                         "Different bit width in merge: "
                         << enumeration.getBitWidth()
                         << " bit value merged to "
                         << getBitWidth() << " bit value");

        mValues.insert(enumeration.mValues.begin(),
                       enumeration.mValues.end());

        if (mValues.size() > mMaxSize)
            setTop();
    }
}

size_t
Enumeration::memoryUsage() const
{
    size_t result = sizeof(Enumeration);
    result += mValues.size() * sizeof(llvm::APInt);
    return result;
}

std::string
Enumeration::toString() const
{
    std::stringstream ss;
    ss << "enumeration";
    if (mTop)
        ss << " top";
    else if (mValues.empty())
        ss << " empty";
    ss << std::endl;

    APIntUtils::USet::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << "    " << Canal::toString(*it) << std::endl;

    return ss.str();
}

void
Enumeration::add(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::operator+, NULL);
}

void
Enumeration::sub(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::operator-, NULL);
}

void
Enumeration::mul(const Domain &a, const Domain &b)
{
#if (LLVM_MAJOR == 2 && LLVM_MINOR < 9)
    applyOperation(a, b, &llvm::APInt::operator*, NULL);
#else
    applyOperation(a, b, NULL, &llvm::APInt::smul_ov);
#endif
}

void
Enumeration::udiv(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::udiv, NULL);
}

void
Enumeration::sdiv(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::sdiv, NULL);
}

void
Enumeration::urem(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::urem, NULL);
}

void
Enumeration::srem(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::srem, NULL);
}

void
Enumeration::shl(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::shl, NULL);
}

void
Enumeration::lshr(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::lshr, NULL);
}

void
Enumeration::ashr(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::ashr, NULL);
}

void
Enumeration::and_(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::operator&, NULL);
}

void
Enumeration::or_(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::operator|, NULL);
}

void
Enumeration::xor_(const Domain &a, const Domain &b)
{
    applyOperation(a, b, &llvm::APInt::operator^, NULL);
}

static bool
intersects(const Enumeration &a,
           const Enumeration &b)
{
    // Signed and unsigned does not matter, it contains specific values
    APIntUtils::USet::const_iterator ita = a.mValues.begin(),
        itb = b.mValues.begin();

    // Algorithm inspired by std::set_intersection
    while (ita != a.mValues.end() && itb != b.mValues.end())
    {
        if ((*ita).slt(*itb))
            ++ita;
        else if ((*itb).slt(*ita))
            ++itb;
        else
            return true;
    }

    return false;
}

void
Enumeration::icmp(const Domain &a, const Domain &b,
                  llvm::CmpInst::Predicate predicate)
{
    const Enumeration &aa = dynCast<const Enumeration&>(a),
        &bb = dynCast<const Enumeration&>(b);

    if (aa.isTop() || bb.isTop())
    {
        setTop(); // Could be both
        return;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom(); // Undefined
        return;
    }

    setBottom();
    mBitWidth = 1;
    llvm::APInt minA, maxA, minB, maxB;
    aa.signedMin(minA);
    aa.signedMax(maxA);
    bb.signedMin(minB);
    bb.signedMax(maxB);
    //Assert: aa.getBitWidth() = bb.getBitWidth()
    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If both enumerations are equal, the result is 1.  If
        // enumeration intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b || (aa.mValues.size() == 1 && aa.mValues == bb.mValues))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (intersects(aa, bb))
            setTop();
        else
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If both enumerations are equal, the result is 0.  If
        // enumeration intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).
        if (!intersects(aa, bb))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (&a == &b || (aa.mValues.size() == 1 && aa.mValues == bb.mValues))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        // If the lowest element from the first enumeration is
        // unsigned greater than the largest element from the second
        // enumeration, the result is 1.  If the largest element from
        // the first enumeration is unsigned lower than the lowest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.begin()->ugt(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.rbegin()->ult(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the largest element from the first enumeration is
        // unsigned lower or equal the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is unsigned larger or equal than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.begin()->uge(*bb.mValues.rbegin()) || &a == &b)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.rbegin()->ule(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first enumeration is
        // unsigned lower than the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is unsigned larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.rbegin()->ult(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.begin()->ugt(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first enumeration is
        // unsigned lower or equal the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is unsigned larger or equal than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.rbegin()->ule(*bb.mValues.begin()) || &a == &b)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.begin()->uge(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        // If the lowest element from the first enumeration is
        // signed greater than the largest element from the second
        // enumeration, the result is 1.  If the largest element from
        // the first enumeration is signed lower than the lowest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (minA.sgt(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (maxA.slt(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first enumeration is
        // signed greater or equal than the largest element from the second
        // enumeration, the result is 1.  If the largest element from
        // the first enumeration is signed lower or equal than the lowest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (minA.sge(maxB) || &a == &b)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (maxA.sle(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first enumeration is
        // signed lower than the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is signed larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (maxA.slt(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (minA.sgt(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first enumeration is
        // signed lower or equal the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is signed larger or equal than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (maxA.sle(minB) || &a == &b)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (minA.sge(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    default:
        CANAL_DIE();
    }
}

void
Enumeration::fcmp(const Domain &a, const Domain &b,
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
        mTop = false;
        mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        break;
    case 1:
        mTop = false;
        mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        break;
    case 2:
        setTop();
        break;
    default:
        CANAL_DIE();
    }
}

void
Enumeration::trunc(const Domain &value)
{
    const Enumeration &enumeration = dynCast<const Enumeration&>(value);
    mTop = enumeration.mTop;
    APIntUtils::USet::const_iterator it = enumeration.mValues.begin();
    for (; it != enumeration.mValues.end(); ++it)
        mValues.insert(APIntUtils::trunc(*it, getBitWidth()));
}

void
Enumeration::zext(const Domain &value)
{
    const Enumeration &enumeration = dynCast<const Enumeration&>(value);
    mTop = enumeration.mTop;
    APIntUtils::USet::const_iterator it = enumeration.mValues.begin();
    for (; it != enumeration.mValues.end(); ++it)
        mValues.insert(APIntUtils::zext(*it, getBitWidth()));
}

void
Enumeration::sext(const Domain &value)
{
    const Enumeration &enumeration = dynCast<const Enumeration&>(value);
    mTop = enumeration.mTop;
    APIntUtils::USet::const_iterator it = enumeration.mValues.begin();
    for (; it != enumeration.mValues.end(); ++it)
        mValues.insert(APIntUtils::sext(*it, getBitWidth()));
}

void
Enumeration::fptoui(const Domain &value)
{
    setTop();
}

void
Enumeration::fptosi(const Domain &value)
{
    setTop();
}

float
Enumeration::accuracy() const
{
    if (mTop)
        return 0;
    // Perfectly accurate.  TODO: consider lowering the accuracy
    // depending on the number of elements.
    return 1;
}

bool
Enumeration::isBottom() const
{
    return mValues.empty() && !mTop;
}

void
Enumeration::setBottom()
{
    mValues.clear();
    mTop = false;
}

bool
Enumeration::isTop() const
{
    return mTop;
}

void
Enumeration::setTop()
{
    mValues.clear();
    mTop = true;
}

void
Enumeration::applyOperation(const Domain &a,
                            const Domain &b,
                            APIntUtils::Operation operation1,
                            APIntUtils::OperationWithOverflow operation2)
{
    const Enumeration &aa = dynCast<const Enumeration&>(a),
        &bb = dynCast<const Enumeration&>(b);

    CANAL_ASSERT(this != &a && this != &b);
    setBottom();
    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return;
    }

    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth());
    APIntUtils::USet::const_iterator aaIt = aa.mValues.begin();
    for (; aaIt != aa.mValues.end(); ++aaIt)
    {
        APIntUtils::USet::const_iterator bbIt = bb.mValues.begin();
        for (; bbIt != bb.mValues.end(); ++bbIt)
        {
            if (operation1)
                mValues.insert(((*aaIt).*(operation1))(*bbIt));
            else
            {
                bool overflow;
                mValues.insert(((*aaIt).*(operation2))(*bbIt, overflow));
                if (overflow)
                {
                    setTop();
                    return;
                }
            }

            if (mValues.size() > mMaxSize)
            {
                setTop();
                return;
            }
        }
    }
}

void
Enumeration::setZero(const llvm::Value *instruction)
{
    mTop = false;
    mValues.clear();
    mValues.insert(llvm::APInt::getNullValue(mBitWidth));
}

} // namespace Integer
} // namespace Canal
