#include "ArrayExactSize.h"
#include "IntegerContainer.h"
#include "IntegerSet.h"
#include "IntegerInterval.h"
#include "Utils.h"
#include "IntegerUtils.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Array {

ExactSize::ExactSize(const Environment &environment,
                     const llvm::SequentialType &type)
    : Domain(environment, Domain::ArrayExactSizeKind),
      mHasExactSize(true),
      mType(type)
{
    uint64_t count = 0;

    const llvm::ArrayType *array = llvm::dyn_cast<llvm::ArrayType>(&type);
    if (array)
        count = array->getNumElements();
    else
    {
        const llvm::VectorType *vector = llvm::dyn_cast<llvm::VectorType>(&type);
        if (vector)
            count = vector->getNumElements();
        else
            mHasExactSize = false;
    }

    for (uint64_t i = 0; i < count; ++i)
    {
        const llvm::Type &elementType = *type.getElementType();
        mValues.push_back(environment.getConstructors().create(elementType));
    }
}

ExactSize::ExactSize(const Environment &environment,
                     const llvm::SequentialType &type,
                     const std::vector<Domain*> &values)
    : Domain(environment, Domain::ArrayExactSizeKind),
      mValues(values),
      mHasExactSize(true),
      mType(type)
{
}

ExactSize::ExactSize(const ExactSize &value)
    : Domain(value), mValues(value.mValues), mHasExactSize(value.mHasExactSize), mType(value.mType)
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

    if (mHasExactSize)
    {
        ss << "arrayExactSize\n";
        std::vector<Domain*>::const_iterator it = mValues.begin(),
            itend = mValues.end();

        for (; it != itend; ++it)
            ss << indent((*it)->toString(), 4);
    }
    else
        ss << "arrayExactSize notExactSize\n";

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

    const ExactSize &array = llvm::cast<ExactSize>(value);
    CANAL_ASSERT(mHasExactSize == array.mHasExactSize);
    CANAL_ASSERT(&mType == &array.mType);
    CANAL_ASSERT(mValues.size() == array.mValues.size());

    std::vector<Domain*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = array.mValues.begin();

    for (; itA != itAend; ++itA, ++itB)
    {
        if (**itA != **itB)
            return false;
    }

    return true;
}

bool
ExactSize::operator<(const Domain& value) const
{
    if (this == &value)
        return false;

    const ExactSize &array = llvm::cast<ExactSize>(value);
    CANAL_ASSERT(mHasExactSize == array.mHasExactSize);
    CANAL_ASSERT(&mType == &array.mType);
    CANAL_ASSERT(mValues.size() == array.mValues.size());

    if (!mHasExactSize)
        return false;

    std::vector<Domain*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = array.mValues.begin();

    for (; itA != itAend; ++itA, ++itB)
    {
        if (!(**itA < **itB))
            return false;
    }

    return true;
}

ExactSize &
ExactSize::join(const Domain &value)
{
    const ExactSize &array = llvm::cast<ExactSize>(value);
    CANAL_ASSERT(mHasExactSize == array.mHasExactSize);
    CANAL_ASSERT(&mType == &array.mType);
    CANAL_ASSERT(mValues.size() == array.mValues.size());

    std::vector<Domain*>::const_iterator
        it = mValues.begin(),
        itend = mValues.end(),
        itv = array.mValues.begin();

    for (; it != itend; ++it, ++itv)
        (*it)->join(**itv);

    return *this;
}

ExactSize &
ExactSize::meet(const Domain &value)
{
    const ExactSize &array = llvm::cast<ExactSize>(value);
    CANAL_ASSERT(mHasExactSize == array.mHasExactSize);
    CANAL_ASSERT(&mType == &array.mType);
    CANAL_ASSERT(mValues.size() == array.mValues.size());

    std::vector<Domain*>::const_iterator
        it = mValues.begin(),
        itend = mValues.end(),
        itv = array.mValues.begin();

    for (; it != itend; ++it, ++itv)
        (*it)->meet(**itv);

    return *this;
}

bool
ExactSize::isBottom() const
{
    if (!mHasExactSize)
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
    CANAL_ASSERT(mHasExactSize);
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setBottom();
}

bool
ExactSize::isTop() const
{
    if (!mHasExactSize)
        return true;

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
    if (!mHasExactSize)
        return 0;

    CANAL_ASSERT(!mValues.empty());
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    float sum = 0;
    for (; it != itend; ++it)
        sum += (*it)->accuracy();

    return sum / mValues.size();
}

static ExactSize &
binaryOperation(ExactSize &result,
                const Domain &a,
                const Domain &b,
                Domain::BinaryOperation operation)
{
    const ExactSize &aa = llvm::cast<ExactSize>(a),
        &bb = llvm::cast<ExactSize>(b);

    CANAL_ASSERT(result.mHasExactSize == aa.mHasExactSize);
    CANAL_ASSERT(&result.mType == &aa.mType);
    CANAL_ASSERT(result.mValues.size() == aa.mValues.size());
    CANAL_ASSERT(result.mHasExactSize == bb.mHasExactSize);
    CANAL_ASSERT(&result.mType == &bb.mType);
    CANAL_ASSERT(result.mValues.size() == bb.mValues.size());

    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    std::vector<Domain*>::iterator it = result.mValues.begin(),
        itend = result.mValues.begin();

    for (; it != itend; ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb);

    return result;
}

ExactSize &
ExactSize::add(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::add);
}

ExactSize &
ExactSize::fadd(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fadd);
}

ExactSize &
ExactSize::sub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sub);
}

ExactSize &
ExactSize::fsub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fsub);
}

ExactSize &
ExactSize::mul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::mul);
}

ExactSize &
ExactSize::fmul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fmul);
}

ExactSize &
ExactSize::udiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::udiv);
}

ExactSize &
ExactSize::sdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sdiv);
}

ExactSize &
ExactSize::fdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fdiv);
}

ExactSize &
ExactSize::urem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::urem);
}

ExactSize &
ExactSize::srem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::srem);
}

ExactSize &
ExactSize::frem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::frem);
}

ExactSize &
ExactSize::shl(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::shl);
}

ExactSize &
ExactSize::lshr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::lshr);
}

ExactSize &
ExactSize::ashr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::ashr);
}

ExactSize &
ExactSize::and_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::and_);
}

ExactSize &
ExactSize::or_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::or_);
}

ExactSize &
ExactSize::xor_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::xor_);
}

static ExactSize &
cmpOperation(ExactSize &result,
             const Domain &a,
             const Domain &b,
             llvm::CmpInst::Predicate predicate,
             Domain::CmpOperation operation)
{
    const ExactSize &aa = llvm::cast<ExactSize>(a),
        &bb = llvm::cast<ExactSize>(b);

    CANAL_ASSERT(result.mHasExactSize == aa.mHasExactSize);
    CANAL_ASSERT(&result.mType == &aa.mType);
    CANAL_ASSERT(result.mValues.size() == aa.mValues.size());
    CANAL_ASSERT(result.mHasExactSize == bb.mHasExactSize);
    CANAL_ASSERT(&result.mType == &bb.mType);
    CANAL_ASSERT(result.mValues.size() == bb.mValues.size());

    std::vector<Domain*>::const_iterator
        it = result.mValues.begin(),
        itend = result.mValues.end(),
        ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != itend; ++it, ++ita, ++itb)
        ((*it)->*(operation))(**ita, **itb, predicate);

    return result;
}

ExactSize &
ExactSize::icmp(const Domain &a,
                const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    return cmpOperation(*this, a, b, predicate, &Domain::icmp);
}

ExactSize &
ExactSize::fcmp(const Domain &a,
                const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    return cmpOperation(*this, a, b, predicate, &Domain::fcmp);
}

Domain *
ExactSize::extractelement(const Domain &index) const
{
    const llvm::Type &elementType = *mType.getElementType();
    Domain *result = mEnvironment.getConstructors().create(elementType);

    if (!mHasExactSize)
    {
        result->setTop();
        return result;
    }

    // First try an enumeration, then interval.
    const Integer::Set &set = Integer::Utils::getSet(index);
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

            result->join(*mValues[numOffset]);
        }

        // At least one of the offsets in the set should point
        // to the array.  Otherwise it might be a bug in the
        // interpreter that requires investigation.
        CANAL_ASSERT_MSG(!result->isBottom() || set.mValues.empty(),
                         "All offsets out of bound, array size "
                         << mValues.size());
        return result;
    }

    const Integer::Interval &interval = Integer::Utils::getInterval(index);
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

        for (uint64_t i = from; i < to; ++i)
            result->join(*mValues[i]);

        return result;
    }

    // Both set and interval are set to the top value, so merge
    // all items of the array.
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        result->join(**it);

    return result;
}

ExactSize &
ExactSize::insertelement(const Domain &array,
                         const Domain &element,
                         const Domain &index)
{
    const ExactSize &exactSize = llvm::cast<ExactSize>(array);
    CANAL_ASSERT(mHasExactSize == exactSize.mHasExactSize);
    CANAL_ASSERT(&mType == &exactSize.mType);
    CANAL_ASSERT(mValues.size() == exactSize.mValues.size());

    if (!mHasExactSize)
        return *this;

    // Copy the original values.
    std::vector<Domain*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = exactSize.mValues.begin();

    for (; itA != itAend; ++itA, ++itB)
        (*itA)->join(**itB);

    // First try an enumeration of indices.
    const Integer::Set &set = Integer::Utils::getSet(index);
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

            if (set.mValues.size() == 1)
            {
                delete mValues[numOffset];
                mValues[numOffset] = element.clone();
            }
            else
                mValues[numOffset]->join(element);
        }

        return *this;
    }

    // Try the interval of indices.
    // Let's care about the unsigned interval only.
    const Integer::Interval &interval = Integer::Utils::getInterval(index);
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
            mValues[loop]->join(element);

        return *this;
    }

    // Both set and interval are set to the top value, so merge
    // the value to all items of the array.
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->join(element);

    return *this;
}

ExactSize &
ExactSize::shufflevector(const Domain &a,
                         const Domain &b,
                         const std::vector<uint32_t> &mask)
{
    if (!mHasExactSize)
        return *this;

    const ExactSize &aa = llvm::cast<ExactSize>(a),
        &bb = llvm::cast<ExactSize>(b);

    CANAL_ASSERT(aa.mHasExactSize == bb.mHasExactSize);
    CANAL_ASSERT(&aa.mType == &bb.mType);
    CANAL_ASSERT(aa.mValues.size() == bb.mValues.size());
    CANAL_ASSERT(mValues.size() == mask.size());

    std::vector<uint32_t>::const_iterator it = mask.begin(),
        itend = mask.end();

    for (; it != itend; ++it)
    {
        if (*it == (uint32_t)-1)
            continue;

        if (*it < aa.mValues.size())
            mValues[itend - it - 1]->join(*aa.mValues[*it]);
        else
        {
            CANAL_ASSERT_MSG(*it < aa.mValues.size() + bb.mValues.size(),
                             "Offset out of bounds.");

            mValues[itend - it - 1]->join(*bb.mValues[*it - aa.mValues.size()]);
        }
    }

    return *this;
}

Domain *
ExactSize::extractvalue(const std::vector<unsigned> &indices) const
{
    if (!mHasExactSize)
    {
        // Cannot use const-correctness here because of differences
        // between LLVM versions.
        llvm::Type *type = (llvm::Type*)&mType;
        std::vector<unsigned>::const_iterator it = indices.begin(),
            itend = indices.end();

        for (; it != itend; ++it)
        {
            llvm::CompositeType *composite = llvm::cast<llvm::CompositeType>(type);
            type = (llvm::Type*)composite->getTypeAtIndex(*it);
        }

        Domain *result = mEnvironment.getConstructors().create(*type);
        result->setTop();
        return result;
    }

    CANAL_ASSERT(!indices.empty());
    unsigned index = indices[0];
    CANAL_ASSERT(index < mValues.size());
    if (indices.size() > 1)
    {
        return mValues[index]->extractvalue(std::vector<unsigned>(indices.begin() + 1,
                                                                  indices.end()));
    }
    else
        return mValues[index]->clone();
}

ExactSize &
ExactSize::insertvalue(const Domain &aggregate,
                       const Domain &element,
                       const std::vector<unsigned> &indices)
{
    const ExactSize &exactSize = llvm::cast<ExactSize>(aggregate);
    CANAL_ASSERT(mHasExactSize == exactSize.mHasExactSize);
    CANAL_ASSERT(&mType == &exactSize.mType);
    CANAL_ASSERT(mValues.size() == exactSize.mValues.size());

    if (!mHasExactSize)
        return *this;

    // Copy the original values.
    std::vector<Domain*>::const_iterator itA = mValues.begin(),
        itAend = mValues.end(),
        itB = exactSize.mValues.begin();

    for (; itA != itAend; ++itA, ++itB)
        (*itA)->join(**itB);

    // Insert the element.
    insertvalue(element, indices);
    return *this;
}

void
ExactSize::insertvalue(const Domain &element,
                       const std::vector<unsigned> &indices)
{
    CANAL_ASSERT(!indices.empty());
    if (!mHasExactSize)
        return;

    unsigned index = indices[0];
    CANAL_ASSERT(index < mValues.size());
    if (indices.size() > 1)
    {
        mValues[index]->insertvalue(element,
                                    std::vector<unsigned>(indices.begin() + 1,
                                                          indices.end()));
    }
    else
    {
        delete mValues[index];
        mValues[index] = element.clone();
    }
}

Domain *
ExactSize::load(const llvm::Type &type,
                const std::vector<Domain*> &offsets) const
{
    if (offsets.empty())
    {
        if (&mType == &type)
            return clone();
        else
        {
            Domain *result = mEnvironment.getConstructors().create(type);
            result->setTop();
            return result;
        }
    }

    if (!mHasExactSize)
    {
        Domain *result = mEnvironment.getConstructors().create(type);
        result->setTop();
        return result;
    }

    Domain *subitem = extractelement(*offsets[0]);
    Domain *result = subitem->load(type, std::vector<Domain*>(offsets.begin() + 1,
                                                              offsets.end()));

    delete subitem;
    return result;
}

ExactSize &
ExactSize::store(const Domain &value,
                 const std::vector<Domain*> &offsets,
                 bool overwrite)
{
    if (!mHasExactSize)
        return *this;

    if (offsets.empty())
        return (ExactSize&)Domain::store(value, offsets, overwrite);

    const Domain &offset = *offsets[0];

    // First try an enumeration, then interval.
    const Integer::Set &set = Integer::Utils::getSet(offset);
    if (!set.isTop())
    {
        APIntUtils::USet::const_iterator it = set.mValues.begin(),
            itend = set.mValues.end();

        if (set.mValues.size() > 1)
            overwrite = false;

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

            mValues[numOffset]->store(value,
                                      std::vector<Domain*>(offsets.begin() + 1,
                                                           offsets.end()),
                                      overwrite);
        }

        return *this;
    }

    const Integer::Interval &interval = Integer::Utils::getInterval(offset);
    // Let's care about the unsigned interval only.
    if (!interval.mUnsignedTop)
    {
        CANAL_ASSERT(interval.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = interval.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = interval.mUnsignedTo.getZExtValue();

        CANAL_ASSERT(from <= to);
        if (to >= mValues.size())
            to = mValues.size();

        if (to - from != 0)
            overwrite = false;

        for (uint64_t i = from; i <= to; ++i)
        {
            mValues[i]->store(value,
                              std::vector<Domain*>(offsets.begin() + 1,
                                                   offsets.end()),
                              overwrite);
        }

        return *this;
    }

    // Both set and interval are set to the top value, so merge
    // the value to all items of the array.
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    if (mValues.size() > 1)
        overwrite = false;

    for (; it != itend; ++it)
    {
        (**it).store(value,
                     std::vector<Domain*>(offsets.begin() + 1,
                                          offsets.end()),
                     overwrite);
    }

    return *this;
}

} // namespace Array
} // namespace Canal
