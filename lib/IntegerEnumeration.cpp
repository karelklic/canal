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
    return mTop == enumeration->mTop && mValues == enumeration->mValues;
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
    ss << "Integer::Enumeration: [";
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
    applyOperation(a, b, &llvm::APInt::operator+);
}

void
Enumeration::sub(const Value &a, const Value &b)
{
    applyOperation(a, b, &llvm::APInt::operator-);
}

void
Enumeration::mul(const Value &a, const Value &b)
{
    setTop();
    //applyOperation(a, b, &llvm::APInt::smul);
}

void
Enumeration::udiv(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::sdiv(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::urem(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::srem(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::shl(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::lshr(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::ashr(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::and_(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::or_(const Value &a, const Value &b)
{
    setTop();
}

void
Enumeration::xor_(const Value &a, const Value &b)
{
    setTop();
}

float
Enumeration::accuracy() const
{
    if (mTop)
        return 0;
    // Perfectly accurate.
    // TODO: consider lowering the accuracy depending on the number of elements.
    return 1;
}

bool
Enumeration::isBottom() const
{
    return mValues.empty() && !mTop;
}

void
Enumeration::setTop()
{
    mValues.clear();
    mTop = true;
}

void Enumeration::applyOperation(const Value &a, const Value &b,
                                 llvm::APInt(llvm::APInt::*operation)(const llvm::APInt&) const)
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
            mValues.insert(((*aaIt).*(operation))(*bbIt));

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
