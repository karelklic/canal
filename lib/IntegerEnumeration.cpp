#include "IntegerEnumeration.h"
#include "Constant.h"
#include "Utils.h"
#include "FloatRange.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Enumeration::Enumeration(unsigned numBits) : mTop(false),
                                             mNumBits(numBits)
{
}

Enumeration::Enumeration(const llvm::APInt &number) : mTop(false)
{
    mValues.insert(number);
}

bool
Enumeration::signedMin(llvm::APInt &result) const
{
    if (mValues.empty())
        return false;

    if (mTop)
        result = llvm::APInt::getSignedMinValue(mNumBits);
    else
    {
        // We assume the set is sorted by unsigned comparison.
        APIntUtils::USet::const_iterator it = mValues.begin();
        result = *it++;
        for (; it != mValues.end(); ++it)
        {
            if (it->slt(result))
                result = *it;
        }
    }

    return true;
}

bool
Enumeration::signedMax(llvm::APInt &result) const
{
    if (mValues.empty())
        return false;

    if (mTop)
        result = llvm::APInt::getSignedMaxValue(mNumBits);
    else
    {
        // We assume the set is sorted by unsigned comparison.
        APIntUtils::USet::const_iterator it = mValues.begin();
        result = *it++;
        for (; it != mValues.end(); ++it)
        {
            if (it->sgt(result))
                result = *it;
        }
    }

    return true;
}

bool
Enumeration::unsignedMin(llvm::APInt &result) const
{
    if (mValues.empty())
        return false;

    if (mTop)
        result = llvm::APInt::getMinValue(mNumBits);
    else
        // We assume the set is sorted by unsigned comparison.
        result = *mValues.begin();

    return true;
}

bool
Enumeration::unsignedMax(llvm::APInt &result) const
{
    if (mValues.empty())
        return false;

    if (mTop)
        result = llvm::APInt::getMaxValue(mNumBits);
    else
        // We assume the set is sorted by unsigned comparison.
        result = *mValues.rbegin();

    return true;
}

Enumeration *
Enumeration::clone() const
{
    return new Enumeration(*this);
}

Enumeration *
Enumeration::cloneCleaned() const
{
    return new Enumeration(getBitWidth());
}

bool
Enumeration::operator==(const Value& value) const
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
Enumeration::merge(const Value &value)
{
    if (mTop)
        return;

    const Constant *constant = dynCast<const Constant*>(&value);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        mValues.insert(constant->getAPInt());
        return;
    }

    const Enumeration &enumeration =
        dynCast<const Enumeration&>(value);

    mValues.insert(enumeration.mValues.begin(),
                   enumeration.mValues.end());
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
Enumeration::add(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::operator+, NULL);
}

void
Enumeration::sub(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::operator-, NULL);
}

void
Enumeration::mul(const Value &a, const Value &b)
{
#if (LLVM_MAJOR == 2 && LLVM_MINOR < 9)
    applyOperation(a, b, &llvm::APInt::operator*, NULL);
#else
    applyOperation(a, b, NULL, &llvm::APInt::smul_ov);
#endif
}

void
Enumeration::udiv(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::udiv, NULL);
}

void
Enumeration::sdiv(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::sdiv, NULL);
}

void
Enumeration::urem(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::urem, NULL);
}

void
Enumeration::srem(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::srem, NULL);
}

void
Enumeration::shl(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::shl, NULL);
}

void
Enumeration::lshr(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::lshr, NULL);
}

void
Enumeration::ashr(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::ashr, NULL);
}

void
Enumeration::and_(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::operator&, NULL);
}

void
Enumeration::or_(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::operator|, NULL);
}

void
Enumeration::xor_(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::operator^, NULL);
}

void
Enumeration::icmp(const Value &a, const Value &b,
                  llvm::CmpInst::Predicate predicate)
{
    const Enumeration &aa = dynCast<const Enumeration&>(a),
        &bb = dynCast<const Enumeration&>(b);

    if (aa.isTop() || bb.isTop())
    {
        setTop(); //Could be both
        return;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom(); //Undefined
        return;
    }

    //Assert: aa.getBitWidth() = bb.getBitWidth()
    llvm::APInt bound = llvm::APInt::getSignedMinValue(aa.getBitWidth());
    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If both enumerations are equal, the result is 1.  If
        // enumeration intersection is empty, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b || (aa.mValues.size() == 1 && aa.mValues == bb.mValues))
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        else if (aa.intersection(bb)) {
            this->setTop();
        }
        else {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If both enumerations are equal, the result is 0.  If
        // enumeration intersection is empty, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).

        if (!aa.intersection(bb)) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if (&a == &b || (aa.mValues.size() == 1 && aa.mValues == bb.mValues)) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        // If the lowest element from the first enumeration is
        // unsigned greater than the largest element from the second
        // enumeration, the result is 1.  If the largest element from
        // the first enumeration is unsigned lower than the lowest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.begin()).ugt(*bb.mValues.rbegin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.rbegin()).ult(*bb.mValues.begin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        // If the largest element from the first enumeration is
        // unsigned lower or equal the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is unsigned larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.begin()).uge(*bb.mValues.rbegin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.rbegin()).ult(*bb.mValues.begin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        // If the largest element from the first enumeration is
        // unsigned lower than the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is unsigned larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.rbegin()).ult(*bb.mValues.begin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.begin()).ugt(*bb.mValues.rbegin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        // If the largest element from the first enumeration is
        // unsigned lower or equal the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is unsigned larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.rbegin()).ule(*bb.mValues.begin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.begin()).ugt(*bb.mValues.rbegin())) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than        
        // If the lowest element from the first enumeration is
        // signed greater than the largest element from the second
        // enumeration, the result is 1.  If the largest element from
        // the first enumeration is signed lower than the lowest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.lower_bound(bound)).sgt(*bb.mValues.upper_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.upper_bound(bound)).slt(*bb.mValues.lower_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        // If the lowest element from the first enumeration is
        // signed greater or equal than the largest element from the second
        // enumeration, the result is 1.  If the largest element from
        // the first enumeration is signed lower than the lowest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.lower_bound(bound)).sge(*bb.mValues.upper_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.upper_bound(bound)).slt(*bb.mValues.lower_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        // If the largest element from the first enumeration is
        // signed lower than the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is signed larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.upper_bound(bound)).slt(*bb.mValues.lower_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.lower_bound(bound)).sgt(*bb.mValues.upper_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        // If the largest element from the first enumeration is
        // signed lower or equal the lowest element from the second
        // enumeration, the result is 1.  If the lowest element from
        // the first enumeration is signed larger than the largest
        // element from the second enumeration, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if ((*aa.mValues.upper_bound(bound)).sle(*bb.mValues.lower_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        }
        else if ((*aa.mValues.lower_bound(bound)).sgt(*bb.mValues.upper_bound(bound))) {
            mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        }
        else this->setTop();
        break;
    default:
        CANAL_DIE();
    }
}

void
Enumeration::fcmp(const Value &a, const Value &b,
                  llvm::CmpInst::Predicate predicate)
{
    const Float::Range &aa = dynCast<const Float::Range&>(a),
        &bb = dynCast<const Float::Range&>(b);

    int result = aa.compare(bb, predicate);
    switch (result)
    {
    case -1:
        setBottom();
        break;
    case 0:
        mValues.insert(llvm::APInt(/*numBits*/1, /*val*/0));
        break;
    case 1:
        mValues.insert(llvm::APInt(/*numBits*/1, /*val*/1));
        break;
    case 2:
        setTop();
        break;
    default:
        CANAL_DIE();
    }
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
Enumeration::applyOperation(const Value &a,
                            const Value &b,
                            APIntUtils::Operation operation1,
                            APIntUtils::OperationWithOverflow operation2)
{
    const Enumeration &aa = dynCast<const Enumeration&>(a),
        &bb = dynCast<const Enumeration&>(b);

    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return;
    }

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

            if (mValues.size() > 40)
            {
                setTop();
                return;
            }
        }
    }
}

bool Enumeration::intersection(const Enumeration &a) const {
    //Signed and unsigned does not matter, it contains specific values
    APIntUtils::USet::const_iterator i = this->mValues.begin(), j = a.mValues.begin();
    //Taken from std::set_intersection (algorithm)
    while (i != this->mValues.end() && j != a.mValues.end()) {
        if ((*i).slt(*j)) i++;
        else if ((*j).slt(*j)) j++;
        else return true;
    }
}

} // namespace Integer
} // namespace Canal
