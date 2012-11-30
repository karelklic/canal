#include "IntegerSet.h"
#include "Utils.h"
#include "FloatInterval.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Integer {

unsigned int Set::SET_THRESHOLD = 40;

Set::Set(const Environment &environment,
                         unsigned bitWidth)
    : Domain(environment), mTop(false), mBitWidth(bitWidth)
{
    CANAL_ASSERT(mBitWidth > 0);
}

Set::Set(const Environment &environment,
                         const llvm::APInt &constant)
    : Domain(environment), mTop(false), mBitWidth(constant.getBitWidth())
{
    mValues.insert(constant);
    CANAL_ASSERT(mBitWidth > 0);
}

Set::Set(const Set &value)
    : Domain(value),
      mValues(value.mValues),
      mTop(value.mTop),
      mBitWidth(value.mBitWidth)
{
}

bool
Set::signedMin(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getSignedMinValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        //Find lowest negative number
        APIntUtils::USet::const_iterator bound =
            mValues.lower_bound(llvm::APInt::getSignedMinValue(mBitWidth));

        if (bound == mValues.end())
        {
            // If there is no negative number in this set then
            // the first element in this set is lowest
            result = *mValues.begin();
        }
        else
            result = *bound;
    }

    return true;
}

bool
Set::signedMax(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getSignedMaxValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        //Find lowest negative number
        APIntUtils::USet::const_iterator bound = mValues.lower_bound(llvm::APInt::getSignedMinValue(mBitWidth));
        if (bound == mValues.end() || //If there is no negative number in this set
                bound == mValues.begin())
        { //or first element in this set is negative
            result = *mValues.rbegin(); //then the last element in this set is highest
        }
        else { //There are some positive numbers as well
            result = *(--bound); //then the highest number is the one directly preceeding lowest negative number
        }
    }

    return true;
}

bool
Set::unsignedMin(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getMinValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        // We assume the set is sorted by unsigned comparison.
        result = *mValues.begin();
    }
    return true;
}

bool
Set::unsignedMax(llvm::APInt &result) const
{
    if (mTop)
        result = llvm::APInt::getMaxValue(mBitWidth);
    else
    {
        if (mValues.empty())
            return false;

        // We assume the set is sorted by unsigned comparison.
        result = *mValues.rbegin();
    }
    return true;
}

bool
Set::isConstant() const
{
    return (!mTop && mValues.size() == 1);
}

bool
Set::isTrue() const
{
    return isConstant() &&
        *mValues.begin() == llvm::APInt(1, 1);
}

bool
Set::isFalse() const
{
    return isConstant() &&
        *mValues.begin() == llvm::APInt(1, 0);
}

Set *
Set::clone() const
{
    return new Set(*this);
}

size_t
Set::memoryUsage() const
{
    size_t result = sizeof(Set);
    result += mValues.size() * sizeof(llvm::APInt);
    return result;
}

std::string
Set::toString() const
{
    StringStream ss;
    ss << "set";
    if (mTop)
        ss << " top";
    else if (mValues.empty())
        ss << " empty";
    ss << "\n";

    APIntUtils::USet::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << "    " << Canal::toString(*it) << "\n";

    return ss.str();
}

void
Set::setZero(const llvm::Value *place)
{
    mTop = false;
    mValues.clear();
    mValues.insert(llvm::APInt::getNullValue(mBitWidth));
}

bool
Set::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const Set *set =
        dynCast<const Set*>(&value);

    if (!set)
        return false;

    if (mTop != set->mTop)
        return false;

    if (mTop)
        return true;

    // Compare values only if the top is not set, otherwise we would
    // get false inequality.
    return mValues == set->mValues;
}

bool
Set::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Set::operator>(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

Set &
Set::join(const Domain &value)
{
    if (mTop)
        return *this;

    const Set &set =
        dynCast<const Set&>(value);

    if (set.isTop())
        setTop();
    else
    {
        CANAL_ASSERT_MSG(set.getBitWidth() == getBitWidth(),
                         "Different bit width in merge: "
                         << set.getBitWidth()
                         << " bit value merged to "
                         << getBitWidth() << " bit value");

        mValues.insert(set.mValues.begin(),
                       set.mValues.end());

        if (mValues.size() > SET_THRESHOLD)
            setTop();
    }

    return *this;
}

Set &
Set::meet(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Set::isBottom() const
{
    return mValues.empty() && !mTop;
}

void
Set::setBottom()
{
    mValues.clear();
    mTop = false;
}

bool
Set::isTop() const
{
    return mTop;
}

void
Set::setTop()
{
    mValues.clear();
    mTop = true;
}

float
Set::accuracy() const
{
    if (mTop)
        return 0;
    // Perfectly accurate.  TODO: consider lowering the accuracy
    // depending on the number of elements.
    return 1;
}

Set &
Set::add(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator+, NULL);
}

Set &
Set::sub(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator-, NULL);
}

Set &
Set::mul(const Domain &a, const Domain &b)
{
#if (LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9)
    return applyOperation(a, b, &llvm::APInt::operator*, NULL);
#else
    return applyOperation(a, b, NULL, &llvm::APInt::smul_ov);
#endif
}

Set &
Set::udiv(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::udiv, NULL);
}

Set &
Set::sdiv(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::sdiv, NULL);
}

Set &
Set::urem(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::urem, NULL);
}

Set &
Set::srem(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::srem, NULL);
}

Set &
Set::shl(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::shl, NULL);
}

Set &
Set::lshr(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::lshr, NULL);
}

Set &
Set::ashr(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::ashr, NULL);
}

Set &
Set::and_(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator&, NULL);
}

Set &
Set::or_(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator|, NULL);
}

Set &
Set::xor_(const Domain &a, const Domain &b)
{
    return applyOperation(a, b, &llvm::APInt::operator^, NULL);
}

static bool
intersects(const Set &a,
           const Set &b)
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

Set &
Set::icmp(const Domain &a, const Domain &b,
                  llvm::CmpInst::Predicate predicate)
{
    const Set &aa = dynCast<const Set&>(a),
        &bb = dynCast<const Set&>(b);

    if (aa.isTop() || bb.isTop())
    {
        setTop(); // Could be both
        return *this;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom(); // Undefined
        return *this;
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
        // If both sets are equal, the result is 1.  If
        // set intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.size() == 1 && aa.mValues == bb.mValues)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (intersects(aa, bb))
            setTop();
        else
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If both sets are equal, the result is 0.  If
        // set intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).
        if (!intersects(aa, bb))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.size() == 1 && aa.mValues == bb.mValues)
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        // If the lowest element from the first set is
        // unsigned greater than the largest element from the second
        // set, the result is 1.  If the largest element from
        // the first set is unsigned lower than the lowest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.begin()->ugt(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.rbegin()->ult(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the largest element from the first set is
        // unsigned lower or equal the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is unsigned larger or equal than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.begin()->uge(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.rbegin()->ule(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first set is
        // unsigned lower than the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is unsigned larger than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.rbegin()->ult(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.begin()->ugt(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first set is
        // unsigned lower or equal the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is unsigned larger or equal than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (aa.mValues.rbegin()->ule(*bb.mValues.begin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (aa.mValues.begin()->uge(*bb.mValues.rbegin()))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        // If the lowest element from the first set is
        // signed greater than the largest element from the second
        // set, the result is 1.  If the largest element from
        // the first set is signed lower than the lowest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (minA.sgt(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (maxA.slt(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first set is
        // signed greater or equal than the largest element from the second
        // set, the result is 1.  If the largest element from
        // the first set is signed lower or equal than the lowest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (minA.sge(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (maxA.sle(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first set is
        // signed lower than the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is signed larger than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (maxA.slt(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (minA.sgt(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first set is
        // signed lower or equal the lowest element from the second
        // set, the result is 1.  If the lowest element from
        // the first set is signed larger or equal than the largest
        // element from the second set, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (maxA.sle(minB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/1));
        else if (minA.sge(maxB))
            mValues.insert(llvm::APInt(/*bitWidth*/1, /*val*/0));
        else
            setTop();

        break;
    default:
        CANAL_DIE();
    }

    return *this;
}

Set &
Set::fcmp(const Domain &a, const Domain &b,
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

    return *this;
}

Set &
Set::trunc(const Domain &value)
{
    const Set &set = dynCast<const Set&>(value);
    mTop = set.mTop;
    APIntUtils::USet::const_iterator it = set.mValues.begin();
    for (; it != set.mValues.end(); ++it)
        mValues.insert(APIntUtils::trunc(*it, getBitWidth()));

    return *this;
}

Set &
Set::zext(const Domain &value)
{
    const Set &set = dynCast<const Set&>(value);
    mTop = set.mTop;
    APIntUtils::USet::const_iterator it = set.mValues.begin();
    for (; it != set.mValues.end(); ++it)
        mValues.insert(APIntUtils::zext(*it, getBitWidth()));

    return *this;
}

Set &
Set::sext(const Domain &value)
{
    const Set &set = dynCast<const Set&>(value);
    mTop = set.mTop;
    APIntUtils::USet::const_iterator it = set.mValues.begin();
    for (; it != set.mValues.end(); ++it)
        mValues.insert(APIntUtils::sext(*it, getBitWidth()));

    return *this;
}

Set &
Set::fptoui(const Domain &value)
{
    setTop();
    return *this;
}

Set &
Set::fptosi(const Domain &value)
{
    setTop();
    return *this;
}

const llvm::IntegerType &
Set::getValueType() const
{
    return *llvm::Type::getIntNTy(mEnvironment.getContext(), getBitWidth());
}

Domain *
Set::getValueCell(uint64_t offset) const
{
    Domain *cell = mEnvironment.getConstructors().createInteger(8);
    cell->setTop();
    return cell;
}

void
Set::mergeValueCell(uint64_t offset, const Domain &value)
{
    setTop();
}

Set &
Set::applyOperation(const Domain &a,
                            const Domain &b,
                            APIntUtils::Operation operation1,
                            APIntUtils::OperationWithOverflow operation2)
{
    const Set &aa = dynCast<const Set&>(a),
        &bb = dynCast<const Set&>(b);

    CANAL_ASSERT(this != &a && this != &b);
    setBottom();
    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return *this;
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
                    return *this;
                }
            }

            if (mValues.size() > SET_THRESHOLD)
            {
                setTop();
                return *this;
            }
        }
    }

    return *this;
}

} // namespace Integer
} // namespace Canal
