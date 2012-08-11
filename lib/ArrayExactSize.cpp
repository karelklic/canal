#include "ArrayExactSize.h"
#include "Constant.h"
#include "IntegerContainer.h"
#include "IntegerEnumeration.h"
#include "IntegerRange.h"
#include "Utils.h"
#include <sstream>
#include <llvm/Constants.h>

namespace Canal {
namespace Array {

ExactSize::ExactSize()
{
}

ExactSize::ExactSize(const ExactSize &exactSize)
{
    mValues = exactSize.mValues;
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = (*it)->clone();
}

ExactSize::~ExactSize()
{
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete (*it);
}

ExactSize *
ExactSize::clone() const
{
    return new ExactSize(*this);
}

bool
ExactSize::operator==(const Value &value) const
{
    const ExactSize *array = dynCast<const ExactSize*>(&value);
    if (!array)
        return false;

    if (mValues.size() != array->mValues.size())
        return false;

    std::vector<Value*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = array->mValues.begin();
    for (; itA != itAend; ++itA, ++itB)
    {
        if (*itA != *itB)
            return false;
    }

    return true;
}

void
ExactSize::merge(const Value &value)
{
    const ExactSize &array = dynCast<const ExactSize&>(value);
    CANAL_ASSERT(mValues.size() == array.mValues.size());
    std::vector<Value*>::iterator itA = mValues.begin();
    std::vector<Value*>::const_iterator itAend = mValues.end(),
        itB = array.mValues.begin();
    for (; itA != itAend; ++itA, ++itB)
        (*itA)->merge(**itB);
}

size_t
ExactSize::memoryUsage() const
{
    size_t size = sizeof(ExactSize);
    size += mValues.capacity() * sizeof(Value*);
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
ExactSize::toString() const
{
    std::stringstream ss;
    ss << "arrayExactSize" << std::endl;
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << indent((*it)->toString(), 4);
    return ss.str();
}

void
ExactSize::add(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::fadd(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::sub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::fsub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::mul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::fmul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::udiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::sdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::fdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::urem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::srem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::frem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::shl(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::lshr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::ashr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::and_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::or_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::xor_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::icmp(const Value &a, const Value &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
ExactSize::fcmp(const Value &a, const Value &b,
                llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

std::vector<Value*>
ExactSize::getItem(const Value &offset) const
{
    std::vector<Value*> result;
    if (const Constant *constant = dynCast<const Constant*>(&offset))
    {
        CANAL_ASSERT(constant->isAPInt());
        CANAL_ASSERT(constant->getAPInt().getBitWidth() <= 64);
        uint64_t numOffset = constant->getAPInt().getZExtValue();
        CANAL_ASSERT(numOffset < mValues.size());
        result.push_back(mValues[numOffset]);
        return result;
    }

    const Integer::Container &integer =
        dynCast<const Integer::Container&>(offset);

    // First try an enumeration, then range.
    const Integer::Enumeration &enumeration = integer.getEnumeration();
    if (!enumeration.isTop())
    {
        APIntUtils::USet::const_iterator it = enumeration.mValues.begin(),
            itend = enumeration.mValues.end();
        for (; it != itend; ++it)
        {
            CANAL_ASSERT(it->getBitWidth() <= 64);
            uint64_t numOffset = it->getZExtValue();

            // If some offset from the enumeration points out of the
            // array bounds, we ignore it FOR NOW.  It might be caused
            // either by a bug in the code, or by imprecision of the
            // interpreter.
            if (numOffset >= mValues.size())
                continue;

            result.push_back(mValues[numOffset]);
        }

        // At least one of the offsets in the enumeration should point
        // to the array.  Otherwise it might be a bug in the
        // interpreter that requires investigation.
        CANAL_ASSERT(!result.empty());
        return result;
    }

    const Integer::Range &range = integer.getRange();
    // Let's care about the unsigned range only.
    if (!range.mUnsignedTop)
    {
        CANAL_ASSERT(range.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = range.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = range.mUnsignedTo.getZExtValue();
        // At least part of the range should point to the array.
        // Otherwise it might be a bug in the interpreter that
        // requires investigation.
        CANAL_ASSERT(from < mValues.size());
        if (to >= mValues.size())
            to = mValues.size();
        result.insert(result.end(), mValues.begin() + from, mValues.begin() + to);
        return result;
    }

    // Both enumeration and range are set to the top value, so return
    // all members.
    result.insert(result.end(), mValues.begin(), mValues.end());

    // Zero length arrays are not supported.
    CANAL_ASSERT(!result.empty());
    return result;
}

Value *
ExactSize::getItem(uint64_t offset) const
{
    CANAL_ASSERT_MSG(offset < mValues.size(),
                     "Offset out of bounds.");
    return mValues[offset];
}

void
ExactSize::setItem(const Value &offset, const Value &value)
{
    const Constant *constant = dynCast<const Constant*>(&offset);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        CANAL_ASSERT(constant->getAPInt().getBitWidth() <= 64);
        uint64_t numOffset = constant->getAPInt().getZExtValue();
        CANAL_ASSERT(numOffset < mValues.size());
        mValues[numOffset]->merge(value);
        return;
    }

    const Integer::Container &integer =
        dynCast<const Integer::Container&>(offset);

    // First try an enumeration, then range.
    const Integer::Enumeration &enumeration = integer.getEnumeration();
    if (!enumeration.isTop())
    {
        APIntUtils::USet::const_iterator it = enumeration.mValues.begin(),
            itend = enumeration.mValues.end();
        for (; it != itend; ++it)
        {
            CANAL_ASSERT(it->getBitWidth() <= 64);
            uint64_t numOffset = it->getZExtValue();

            // If some offset from the enumeration points out of the
            // array bounds, we ignore it.  It might be caused either
            // by a bug in the code, or by imprecision of the
            // interpreter.
            if (numOffset >= mValues.size())
                continue;

            mValues[numOffset]->merge(value);
        }
        return;
    }

    const Integer::Range &range = integer.getRange();
    // Let's care about the unsigned range only.
    if (!range.mUnsignedTop)
    {
        CANAL_ASSERT(range.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = range.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = range.mUnsignedTo.getZExtValue();
        // At least part of the range should point to the array.
        // Otherwise it might be a bug in the interpreter that
        // requires investigation.
        CANAL_ASSERT(from < mValues.size());
        for (size_t loop = from; loop < mValues.size() && loop <= to; ++loop)
            mValues[loop]->merge(value);
        return;
    }

    // Both enumeration and range are set to the top value, so merge
    // the value to all items of the array.
    std::vector<Value*>::const_iterator it = mValues.begin(),
        itend = mValues.end();
    for (; it != itend; ++it)
        (*it)->merge(value);
}

void
ExactSize::setItem(uint64_t offset, const Value &value)
{
    CANAL_ASSERT_MSG(offset < mValues.size(),
                     "Offset out of bounds.");
    mValues[offset]->merge(value);
}

} // namespace Array
} // namespace Canal
