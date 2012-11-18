#include "ArrayExactSize.h"
#include "IntegerContainer.h"
#include "IntegerEnumeration.h"
#include "IntegerInterval.h"
#include "Utils.h"
#include <sstream>

namespace Canal {
namespace Array {

ExactSize::ExactSize(const Environment &environment,
                     uint64_t size,
                     const Domain &value)
    : Domain(environment)
{
    for (uint64_t i = 0; i < size; ++i)
        mValues.push_back(value.clone());
}

ExactSize::ExactSize(const Environment &environment,
                     const std::vector<Domain*> &values)
    : Domain(environment), mValues(values)
{
}

ExactSize::ExactSize(const ExactSize &value)
    : Domain(value)
{
    mValues = value.mValues;
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = (*it)->clone();
}

ExactSize::~ExactSize()
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete (*it);
}

ExactSize *
ExactSize::clone() const
{
    return new ExactSize(*this);
}

bool
ExactSize::operator==(const Domain &value) const
{
    const ExactSize *array = dynCast<const ExactSize*>(&value);
    if (!array)
        return false;

    if (mValues.size() != array->mValues.size())
        return false;

    std::vector<Domain*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = array->mValues.begin();

    for (; itA != itAend; ++itA, ++itB)
    {
        if (**itA != **itB)
            return false;
    }

    return true;
}

void
ExactSize::merge(const Domain &value)
{
    const ExactSize &array = dynCast<const ExactSize&>(value);
    CANAL_ASSERT(mValues.size() == array.mValues.size());
    std::vector<Domain*>::iterator itA = mValues.begin();
    std::vector<Domain*>::const_iterator itAend = mValues.end(),
        itB = array.mValues.begin();

    for (; itA != itAend; ++itA, ++itB)
        (*itA)->merge(**itB);
}

size_t
ExactSize::memoryUsage() const
{
    size_t size = sizeof(ExactSize);
    size += mValues.capacity() * sizeof(Domain*);
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
ExactSize::toString() const
{
    std::stringstream ss;
    ss << "arrayExactSize" << std::endl;
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << indent((*it)->toString(), 4);

    return ss.str();
}

static void
binaryOperation(ExactSize &result,
                const Domain &a,
                const Domain &b,
                void(Domain::*operation)(const Domain&, const Domain&))
{
    const ExactSize &aa = dynCast<const ExactSize&>(a),
        &bb = dynCast<const ExactSize&>(b);

    CANAL_ASSERT_MSG(aa.size() == bb.size() && result.size() == aa.size(),
                     "Binary operations with arrays "
                     "require the array size to be equal.");

    std::vector<Domain*>::const_iterator itA = aa.mValues.begin(),
        itB = bb.mValues.begin();

    std::vector<Domain*>::iterator it = result.mValues.begin();
    for (; it != result.mValues.end(); ++it, ++itA, ++itB)
        ((**it).*(operation))(**itA, **itB);
}

void
ExactSize::add(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::add);
}

void
ExactSize::fadd(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::fadd);
}

void
ExactSize::sub(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::sub);
}

void
ExactSize::fsub(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::fsub);
}

void
ExactSize::mul(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::mul);
}

void
ExactSize::fmul(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::fmul);
}

void
ExactSize::udiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::udiv);
}

void
ExactSize::sdiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::sdiv);
}

void
ExactSize::fdiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::fdiv);
}

void
ExactSize::urem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::urem);
}

void
ExactSize::srem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::srem);
}

void
ExactSize::frem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::frem);
}

void
ExactSize::shl(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::shl);
}

void
ExactSize::lshr(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::lshr);
}

void
ExactSize::ashr(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::ashr);
}

void
ExactSize::and_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::and_);
}

void
ExactSize::or_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::or_);
}

void
ExactSize::xor_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::xor_);
}

static void
cmpOperation(ExactSize &result,
             const Domain &a,
             const Domain &b,
             llvm::CmpInst::Predicate predicate,
             Domain::CmpOperation operation)
{
    const ExactSize &aa = dynCast<const ExactSize&>(a),
        &bb = dynCast<const ExactSize&>(b);

    CANAL_ASSERT_MSG(result.size() == aa.size() &&
                     aa.size() == bb.size(),
                     "Binary operations with arrays "
                     "require the array size to be equal.");

    std::vector<Domain*>::const_iterator
        itR = result.mValues.begin(),
        itA = aa.mValues.begin(),
        itB = bb.mValues.begin();

    for (; itR != result.mValues.end(); ++itR, ++itA, ++itB)
        ((*itR)->*(operation))(**itA, **itB, predicate);
}

void
ExactSize::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    cmpOperation(*this, a, b, predicate, &Domain::icmp);
}

void
ExactSize::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    cmpOperation(*this, a, b, predicate, &Domain::fcmp);
}

std::vector<Domain*>
ExactSize::getItem(const Domain &offset) const
{
    std::vector<Domain*> result;

    const Integer::Container &integer =
        dynCast<const Integer::Container&>(offset);

    // First try an enumeration, then interval.
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
        CANAL_ASSERT_MSG(!result.empty() || enumeration.mValues.empty(),
                         "All offsets out of bound, array size "
                         << mValues.size());
        return result;
    }

    const Integer::Interval &interval = integer.getInterval();
    // Let's care about the unsigned interval only.
    if (!interval.mUnsignedTop)
    {
        CANAL_ASSERT(interval.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = interval.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = interval.mUnsignedTo.getZExtValue();
        // At least part of the interval should point to the array.
        // Otherwise it might be a bug in the interpreter that
        // requires investigation.
        CANAL_ASSERT(from < mValues.size());
        if (to >= mValues.size())
            to = mValues.size();

        result.insert(result.end(),
                      mValues.begin() + from,
                      mValues.begin() + to);

        return result;
    }

    // Both enumeration and interval are set to the top value, so return
    // all members.
    result.insert(result.end(), mValues.begin(), mValues.end());

    // Zero length arrays are not supported.
    CANAL_ASSERT(!result.empty());
    return result;
}

Domain *
ExactSize::getItem(uint64_t offset) const
{
    CANAL_ASSERT_MSG(offset < mValues.size(),
                     "Offset out of bounds.");
    return mValues[offset];
}

void
ExactSize::setItem(const Domain &offset, const Domain &value)
{
    const Integer::Container &integer =
        dynCast<const Integer::Container&>(offset);

    // First try an enumeration, then interval.
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

    const Integer::Interval &interval = integer.getInterval();
    // Let's care about the unsigned interval only.
    if (!interval.mUnsignedTop)
    {
        CANAL_ASSERT(interval.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = interval.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = interval.mUnsignedTo.getZExtValue();
        // At least part of the interval should point to the array.
        // Otherwise it might be a bug in the interpreter that
        // requires investigation.
        CANAL_ASSERT(from < mValues.size());
        for (size_t loop = from; loop < mValues.size() && loop <= to; ++loop)
            mValues[loop]->merge(value);

        return;
    }

    // Both enumeration and interval are set to the top value, so merge
    // the value to all items of the array.
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->merge(value);
}

void
ExactSize::setItem(uint64_t offset, const Domain &value)
{
    CANAL_ASSERT_MSG(offset < mValues.size(),
                     "Offset out of bounds.");

    mValues[offset]->merge(value);
}

void
ExactSize::setZero(const llvm::Value *place)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
            itend = mValues.end();

    for (; it != itend; it ++)
        (*it)->setZero(place);
}

} // namespace Array
} // namespace Canal
