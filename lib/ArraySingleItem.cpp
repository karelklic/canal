#include "ArraySingleItem.h"
#include "Utils.h"
#include "ProductVector.h"
#include "IntegerUtils.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Array {

SingleItem::SingleItem(const Environment &environment,
                       const llvm::SequentialType &type)
    : Domain(environment, Domain::ArraySingleItemKind),
      mType(type)
{
    const llvm::Type &elementType = *type.getElementType();
    mValue = environment.getConstructors().create(elementType);

    uint64_t count = 0;

    const llvm::ArrayType *array = llvm::dyn_cast<llvm::ArrayType>(&type);
    if (array)
        count = array->getNumElements();

    const llvm::VectorType *vector = llvm::dyn_cast<llvm::VectorType>(&type);
    if (vector)
        count = vector->getNumElements();

    mSize = environment.getConstructors().createInteger(llvm::APInt(64, count));
    if (count == 0)
        mSize->setTop();
}

SingleItem::SingleItem(const Environment &environment,
                       const llvm::SequentialType &type,
                       std::vector<Domain*>::const_iterator begin,
                       std::vector<Domain*>::const_iterator end)
    : Domain(environment, Domain::ArraySingleItemKind), mType(type)
{
    llvm::APInt size(64, end - begin);
    mSize = environment.getConstructors().createInteger(size);

    const llvm::Type &elementType = *type.getElementType();
    mValue = environment.getConstructors().create(elementType);
    std::vector<Domain*>::const_iterator it = begin;
    for (; it != end; ++it)
        mValue->join(**it);
}

SingleItem::SingleItem(const Environment &environment,
                       const llvm::SequentialType &type,
                       Domain *size)
    : Domain(environment, Domain::ArraySingleItemKind),
      mSize(size),
      mType(type)
{
    const llvm::Type &elementType = *type.getElementType();
    mValue = environment.getConstructors().create(elementType);
}

SingleItem::SingleItem(const SingleItem &value)
    : Domain(value), mValue(value.mValue), mSize(value.mSize), mType(value.mType)
{
    if (mValue)
        mValue = mValue->clone();

    if (mSize)
        mSize = mSize->clone();
}

SingleItem::~SingleItem()
{
    delete mValue;
    delete mSize;
}

SingleItem *
SingleItem::clone() const
{
    return new SingleItem(*this);
}

size_t
SingleItem::memoryUsage() const
{
    size_t size = sizeof(SingleItem);
    size += (mValue ? mValue->memoryUsage() : 0);
    size += (mSize ? mSize->memoryUsage() : 0);
    return size;
}

std::string
SingleItem::toString() const
{
    StringStream ss;
    ss << "arraySingleItem\n";
    ss << "    size\n";
    ss << indent(mSize->toString(), 8);
    ss << "    value\n";
    ss << indent(mValue->toString(), 8);
    return ss.str();
}

void
SingleItem::setZero(const llvm::Value *place)
{
    mValue->setZero(place);
}

bool
SingleItem::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const SingleItem &singleItem = checkedCast<SingleItem>(value);
    if ((mSize && !singleItem.mSize) || (!mSize && singleItem.mSize))
        return false;

    if ((mValue && !singleItem.mValue) || (!mValue && singleItem.mValue))
        return false;

    if (mValue && *mValue != *singleItem.mValue)
        return false;

    if (mSize && *mSize != *singleItem.mSize)
        return false;

    return true;
}

bool
SingleItem::operator<(const Domain& value) const
{
    CANAL_NOT_IMPLEMENTED();
}

SingleItem &
SingleItem::join(const Domain &value)
{
    const SingleItem &array = llvm::cast<SingleItem>(value);
    mValue->join(*array.mValue);
    mSize->join(*array.mSize);
    return *this;
}

SingleItem &
SingleItem::meet(const Domain &value)
{
    const SingleItem &array = llvm::cast<SingleItem>(value);
    mValue->meet(*array.mValue);
    mSize->meet(*array.mSize);
    return *this;
}

bool
SingleItem::isBottom() const
{
    return mValue->isBottom();
}

void
SingleItem::setBottom()
{
    mValue->setBottom();
}

bool
SingleItem::isTop() const
{
    return mValue->isTop();
}

void
SingleItem::setTop()
{
    mValue->setTop();
}

float
SingleItem::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

static SingleItem &
binaryOperation(SingleItem &result,
                const Domain &a,
                const Domain &b,
                Domain::BinaryOperation operation)
{
    const SingleItem &aa = llvm::cast<SingleItem>(a),
        &bb = llvm::cast<SingleItem>(b);

    ((result.mValue)->*(operation))(*aa.mValue, *bb.mValue);
    return result;
}

SingleItem &
SingleItem::add(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::add);
}

SingleItem &
SingleItem::fadd(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fadd);
}

SingleItem &
SingleItem::sub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sub);
}

SingleItem &
SingleItem::fsub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fsub);
}

SingleItem &
SingleItem::mul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::mul);
}

SingleItem &
SingleItem::fmul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fmul);
}

SingleItem &
SingleItem::udiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::udiv);
}

SingleItem &
SingleItem::sdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sdiv);
}

SingleItem &
SingleItem::fdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::fdiv);
}

SingleItem &
SingleItem::urem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::urem);
}

SingleItem &
SingleItem::srem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::srem);
}

SingleItem &
SingleItem::frem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::frem);
}

SingleItem &
SingleItem::shl(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::shl);
}

SingleItem &
SingleItem::lshr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::lshr);
}

SingleItem &
SingleItem::ashr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::ashr);
}

SingleItem &
SingleItem::and_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::and_);
}

SingleItem &
SingleItem::or_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::or_);
}

SingleItem &
SingleItem::xor_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::xor_);
}

SingleItem &
SingleItem::icmp(const Domain &a,
                 const Domain &b,
                 llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

SingleItem &
SingleItem::fcmp(const Domain &a,
                 const Domain &b,
                 llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain *
SingleItem::extractelement(const Domain &index) const
{
    return mValue->clone();
}

SingleItem &
SingleItem::insertelement(const Domain &array,
                          const Domain &element,
                          const Domain &index)
{
    const SingleItem &singleItem = llvm::cast<SingleItem>(array);
    CANAL_ASSERT(&mType == &singleItem.mType);
    mValue->join(*singleItem.mValue);
    mValue->join(element);

    delete mSize;
    mSize = singleItem.mSize->clone();
    return *this;
}

SingleItem &
SingleItem::shufflevector(const Domain &a,
                          const Domain &b,
                          const std::vector<uint32_t> &mask)
{
    const SingleItem &aa = llvm::cast<SingleItem>(a),
        &bb = llvm::cast<SingleItem>(b);

    CANAL_ASSERT(&aa.mType == &bb.mType);
    mValue->join(*aa.mValue);
    mValue->join(*bb.mValue);
    return *this;
}

Domain *
SingleItem::extractvalue(const std::vector<unsigned> &indices) const
{
    CANAL_ASSERT(!indices.empty());
    if (indices.size() > 1)
    {
        return mValue->extractvalue(std::vector<unsigned>(indices.begin() + 1,
                                                          indices.end()));
    }
    else
        return mValue->clone();
}

SingleItem &
SingleItem::insertvalue(const Domain &aggregate,
                        const Domain &element,
                        const std::vector<unsigned> &indices)
{
    const SingleItem &singleItem = llvm::cast<SingleItem>(aggregate);
    CANAL_ASSERT(&mType == &singleItem.mType);
    mValue->join(*singleItem.mValue);
    delete mSize;
    mSize = singleItem.mSize->clone();

    insertvalue(element, indices);
    return *this;
}

void
SingleItem::insertvalue(const Domain &element,
                        const std::vector<unsigned> &indices)
{
    CANAL_ASSERT(!indices.empty());
    if (indices.size() > 1)
    {
        mValue->insertvalue(element,
                            std::vector<unsigned>(indices.begin() + 1,
                                                  indices.end()));
    }
    else
        mValue->join(element);
}

Domain *
SingleItem::load(const llvm::Type &type,
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

    Domain *subitem = extractelement(*offsets[0]);
    Domain *result = subitem->load(type, std::vector<Domain*>(offsets.begin() + 1,
                                                              offsets.end()));

    delete subitem;
    return result;
}

SingleItem &
SingleItem::store(const Domain &value,
                  const std::vector<Domain*> &offsets,
                  bool overwrite)
{
    if (offsets.empty())
        return (SingleItem&)Domain::store(value, offsets, overwrite);

    mValue->store(value,
                  std::vector<Domain*>(offsets.begin() + 1,
                                       offsets.end()),
                  false);

    return *this;
}

} // namespace Array
} // namespace Canal
