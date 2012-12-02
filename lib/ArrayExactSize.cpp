#include "ArrayExactSize.h"
#include "IntegerContainer.h"
#include "IntegerSet.h"
#include "IntegerInterval.h"
#include "Utils.h"
#include "Environment.h"
#include "Constructors.h"
#include "IntegerUtils.h"

namespace Canal {
namespace Array {

ExactSize::ExactSize(const Environment &environment,
                     const llvm::SequentialType &type)
    : Domain(environment), mType(type)
{
    uint64_t count = 0;

    const llvm::ArrayType *array = llvm::dyn_cast<llvm::ArrayType>(&type);
    if (array)
        count = array->getNumElements();

    const llvm::VectorType *vector = llvm::dyn_cast<llvm::VectorType>(&type);
    if (vector)
        count = vector->getNumElements();

    for (uint64_t i = 0; i < count; ++i)
    {
        const llvm::Type &elementType = *type.getElementType();
        mValues.push_back(environment.getConstructors().create(elementType));
    }
}

ExactSize::ExactSize(const Environment &environment,
                     const llvm::SequentialType &type,
                     const std::vector<Domain*> &values)
    : Domain(environment), mValues(values), mType(type)
{
}

ExactSize::ExactSize(const Environment &environment,
                     const llvm::SequentialType &type,
                     Domain *size)
    : Domain(environment), mType(type)
{
    delete size;
}

ExactSize::ExactSize(const ExactSize &value)
    : Domain(value), mValues(value.mValues), mType(value.mType)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        *it = (*it)->clone();
}

ExactSize::~ExactSize()
{
    llvm::DeleteContainerPointers(mValues);
}

ExactSize *
ExactSize::clone() const
{
    return new ExactSize(*this);
}

size_t
ExactSize::memoryUsage() const
{
    size_t size = sizeof(ExactSize);
    size += mValues.capacity() * sizeof(Domain*);
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
ExactSize::toString() const
{
    StringStream ss;
    ss << "arrayExactSize\n";
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        ss << indent((*it)->toString(), 4);

    return ss.str();
}

void
ExactSize::setZero(const llvm::Value *place)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
            itend = mValues.end();

    for (; it != itend; it ++)
        (*it)->setZero(place);
}

bool
ExactSize::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

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
        if (*itA == *itB) //If iterators point to the same object
            continue; //skip casting and comparison of object
        if (**itA != **itB)
            return false;
    }

    return true;
}

bool
ExactSize::operator<(const Domain& value) const
{
    CANAL_NOT_IMPLEMENTED();
}

ExactSize &
ExactSize::join(const Domain &value)
{
    const ExactSize &val = dynCast<const ExactSize&>(value);

    CANAL_ASSERT_MSG(val.size() == size(),
                     "Operations with arrays "
                     "require the array size to be equal.");

    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end(),
        itv = val.mValues.begin();

    for (; it != itend; ++it, ++itv)
        (*it)->join(**itv);

    return *this;
}

ExactSize &
ExactSize::meet(const Domain &value)
{
    const ExactSize &val = dynCast<const ExactSize&>(value);

    CANAL_ASSERT_MSG(val.size() == size(),
                     "Operations with arrays "
                     "require the array size to be equal.");

    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end(),
        itv = val.mValues.begin();

    for (; it != itend; ++it, ++itv)
        (*it)->meet(**itv);

    return *this;
}

bool
ExactSize::isBottom() const
{
    if (mValues.empty())
        return false;

    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
    {
        if (!(*it)->isBottom())
            return false;
    }

    return true;
}

void
ExactSize::setBottom()
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setBottom();
}

bool
ExactSize::isTop() const
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
    {
        if (!(*it)->isTop())
            return false;
    }

    return true;
}

void
ExactSize::setTop()
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setTop();
}

float
ExactSize::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

ExactSize &
ExactSize::add(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::add);
}

ExactSize &
ExactSize::fadd(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::fadd);
}

ExactSize &
ExactSize::sub(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::sub);
}

ExactSize &
ExactSize::fsub(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::fsub);
}

ExactSize &
ExactSize::mul(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::mul);
}

ExactSize &
ExactSize::fmul(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::fmul);
}

ExactSize &
ExactSize::udiv(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::udiv);
}

ExactSize &
ExactSize::sdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::sdiv);
}

ExactSize &
ExactSize::fdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::fdiv);
}

ExactSize &
ExactSize::urem(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::urem);
}

ExactSize &
ExactSize::srem(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::srem);
}

ExactSize &
ExactSize::frem(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::frem);
}

ExactSize &
ExactSize::shl(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::shl);
}

ExactSize &
ExactSize::lshr(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::lshr);
}

ExactSize &
ExactSize::ashr(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::ashr);
}

ExactSize &
ExactSize::and_(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::and_);
}

ExactSize &
ExactSize::or_(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::or_);
}

ExactSize &
ExactSize::xor_(const Domain &a, const Domain &b)
{
    return binaryOperation(a, b, &Domain::xor_);
}

ExactSize &
ExactSize::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    return cmpOperation(a, b, predicate, &Domain::icmp);
}

ExactSize &
ExactSize::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    return cmpOperation(a, b, predicate, &Domain::fcmp);
}

const llvm::SequentialType &
ExactSize::getValueType() const
{
    return mType;
}

Domain *
ExactSize::getValueCell(uint64_t offset) const
{
    Domain *cell = mEnvironment.getConstructors().createInteger(8);
    cell->setTop();
    return cell;
}

void
ExactSize::mergeValueCell(uint64_t offset, const Domain &value)
{
    setTop();
}


std::vector<Domain*>
ExactSize::getItem(const Domain &offset) const
{
    std::vector<Domain*> result;

    // First try an enumeration, then interval.
    const Integer::Set &set = Integer::Utils::getSet(offset);
    if (!set.isTop())
    {
        APIntUtils::USet::const_iterator it = set.mValues.begin(),
            itend = set.mValues.end();

        for (; it != itend; ++it)
        {
            CANAL_ASSERT(it->getBitWidth() <= 64);
            uint64_t numOffset = it->getZExtValue();

            // If some offset from the set points out of the
            // array bounds, we ignore it FOR NOW.  It might be caused
            // either by a bug in the code, or by imprecision of the
            // interpreter.
            if (numOffset >= mValues.size())
                continue;

            result.push_back(mValues[numOffset]);
        }

        // At least one of the offsets in the set should point
        // to the array.  Otherwise it might be a bug in the
        // interpreter that requires investigation.
        CANAL_ASSERT_MSG(!result.empty() || set.mValues.empty(),
                         "All offsets out of bound, array size "
                         << mValues.size());
        return result;
    }

    const Integer::Interval &interval = Integer::Utils::getInterval(offset);
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

    // Both set and interval are set to the top value, so return
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
    // First try an enumeration, then interval.
    const Integer::Set &set = Integer::Utils::getSet(offset);
    if (!set.isTop())
    {
        APIntUtils::USet::const_iterator it = set.mValues.begin(),
            itend = set.mValues.end();

        for (; it != itend; ++it)
        {
            CANAL_ASSERT(it->getBitWidth() <= 64);
            uint64_t numOffset = it->getZExtValue();

            // If some offset from the set points out of the
            // array bounds, we ignore it.  It might be caused either
            // by a bug in the code, or by imprecision of the
            // interpreter.
            if (numOffset >= mValues.size())
                continue;

            mValues[numOffset]->join(value);
        }

        return;
    }

    const Integer::Interval &interval = Integer::Utils::getInterval(offset);
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
            mValues[loop]->join(value);

        return;
    }

    // Both set and interval are set to the top value, so merge
    // the value to all items of the array.
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->join(value);
}

void
ExactSize::setItem(uint64_t offset, const Domain &value)
{
    CANAL_ASSERT_MSG(offset < mValues.size(),
                     "Offset out of bounds.");

    mValues[offset]->join(value);
}

ExactSize &
ExactSize::binaryOperation(const Domain &a,
                           const Domain &b,
                           Domain::BinaryOperation operation)
{
    const ExactSize &aa = dynCast<const ExactSize&>(a),
        &bb = dynCast<const ExactSize&>(b);

    CANAL_ASSERT_MSG(aa.size() == bb.size() && size() == aa.size(),
                     "Binary operations with arrays "
                     "require the array size to be equal.");

    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.begin();

    for (; it != itend; ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb);

    return *this;
}

ExactSize &
ExactSize::cmpOperation(const Domain &a,
                        const Domain &b,
                        llvm::CmpInst::Predicate predicate,
                        Domain::CmpOperation operation)
{
    const ExactSize &aa = dynCast<const ExactSize&>(a),
        &bb = dynCast<const ExactSize&>(b);

    CANAL_ASSERT_MSG(size() == aa.size() &&
                     aa.size() == bb.size(),
                     "Binary operations with arrays "
                     "require the array size to be equal.");

    std::vector<Domain*>::const_iterator
        it = mValues.begin(),
        itend = mValues.end(),
        ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != itend; ++it, ++ita, ++itb)
        ((*it)->*(operation))(**ita, **itb, predicate);

    return *this;
}

} // namespace Array
} // namespace Canal
