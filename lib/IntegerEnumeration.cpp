#include "IntegerEnumeration.h"
#include "Constant.h"
#include "Utils.h"

namespace Canal {
namespace Integer {

Enumeration::Enumeration() : mTop(false)
{
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

void
Enumeration::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Integer:Enumeration()";
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

} // namespace Integer
} // namespace Canal