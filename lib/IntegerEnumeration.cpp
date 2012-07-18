#include "IntegerEnumeration.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Enumeration::Enumeration() : mTop(false)
{
}

Enumeration::Enumeration(const llvm::APInt &number) : mTop(false)
{
    mValues.insert(number);
}

llvm::APInt
Enumeration::signedMin() const
{
    if (mTop)
        return llvm::APInt::getSignedMinValue();

    // We assume the set is sorted by unsigned comparison.
    APIntSet::const_iterator it = mValues.begin();
    llvm::APInt lowest = *it++;
    for (; it != mValues.end(); ++it)
    {
        if (it->slt(lowest))
            lowest = *it;
    }

    return lowest;
}

llvm::APInt
Enumeration::signedMax() const
{
    if (mTop)
        return llvm::APInt::getSignedMaxValue();

    // We assume the set is sorted by unsigned comparison.
    APIntSet::const_iterator it = mValues.begin();
    llvm::APInt highest = *it++;
    for (; it != mValues.end(); ++it)
    {
        if (it->sgt(highest))
            highest = *it;
    }

    return highest;
}

llvm::APInt
Enumeration::unsignedMin() const
{
    if (mTop)
        return llvm::APInt::getMinValue();

    // We assume the set is sorted by unsigned comparison.
    return mValues.begin();
}

llvm::APInt
Enumeration::unsignedMax() const
{
    if (mTop)
        return llvm::APInt::getMaxValue();

    // We assume the set is sorted by unsigned comparison.
    return mValues.rbegin();
}

Enumeration *
Enumeration::clone() const
{
    return new Enumeration(*this);
}

bool
Enumeration::operator==(const Value& value) const
{
    const Enumeration *enumeration = dynamic_cast<const Enumeration*>(&value);
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

    const Constant *constant = dynamic_cast<const Constant*>(&value);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        mValues.insert(constant->getAPInt());
        return;
    }

    const Enumeration &enumeration = dynamic_cast<const Enumeration&>(value);
    mValues.insert(enumeration.mValues.begin(), enumeration.mValues.end());
}

size_t
Enumeration::memoryUsage() const
{
    size_t result = sizeof(Enumeration);
    result += mValues.size() * sizeof(llvm::APInt);
    return result;
}

std::string
Enumeration::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Integer::Enumeration: [" << std::endl;
    for (std::set<llvm::APInt, APIntComp>::const_iterator it = mValues.begin(); it != mValues.end(); ++it)
    {
        ss << "    " << Canal::toString(*it) << std::endl;
    }

    ss << "]";
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

float
Enumeration::accuracy() const
{
    if (mTop)
        return 0;
    // Perfectly accurate.
    // TODO: consider lowering the accuracy depending on the number of
    // elements.
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
                            APIntOperation operation1,
                            APIntOperationWithOverflow operation2)
{
    const Enumeration &aa = dynamic_cast<const Enumeration&>(a),
        &bb = dynamic_cast<const Enumeration&>(b);

    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return;
    }

    APIntSet::const_iterator aaIt = aa.mValues.begin();
    for (; aaIt != aa.mValues.end(); ++aaIt)
    {
        APIntSet::const_iterator bbIt = bb.mValues.begin();
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

} // namespace Integer
} // namespace Canal
