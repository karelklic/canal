#include "ProductVector.h"
#include "ProductMessage.h"
#include "IntegerBitfield.h"
#include "IntegerSet.h"
#include "IntegerInterval.h"
#include "Environment.h"
#include "Constructors.h"
#include "Utils.h"
#include "APIntUtils.h"
#include "MemoryPointer.h"

namespace Canal {
namespace Product {

Vector::Vector(const Environment &environment)
    : Domain(environment, Domain::ProductVectorKind)
{
}

Vector::Vector(const Vector &value)
    : Domain(value)
{
    mValues = value.mValues;
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = (*it)->clone();
}

Vector::~Vector()
{
    llvm::DeleteContainerPointers(mValues);
}

Vector *
Vector::clone() const
{
    return new Vector(*this);
}

size_t
Vector::memoryUsage() const
{
    size_t size(0);
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();
    return size;
}

std::string
Vector::toString() const
{
    StringStream ss;
    ss << "productVector\n";
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        ss << indent((*it)->toString(), 4);

    return ss.str();
}

void
Vector::setZero(const llvm::Value *place)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setZero(place);
}

bool
Vector::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const Vector &container = checkedCast<Vector>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator ita = mValues.begin(),
        itb = container.mValues.begin();

    for (; ita != mValues.end(); ++ita, ++itb)
    {
        if (**ita != **itb)
            return false;
    }

    return true;
}

bool
Vector::operator<(const Domain &value) const
{
    if (this == &value)
        return false;

    const Vector &container = checkedCast<Vector>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator ita = mValues.begin(),
        itb = container.mValues.begin();

    for (; ita != mValues.end(); ++ita, ++itb)
    {
        if (!(**ita < **itb))
            return false;
    }

    return true;
}

Vector &
Vector::join(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    const Vector &container = checkedCast<Vector>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->join(**it2);

    collaborate();

    return *this;
}

Vector &
Vector::meet(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    const Vector &container = checkedCast<Vector>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->meet(**it2);

    collaborate();

    return *this;
}

bool
Vector::isBottom() const
{
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
Vector::setBottom()
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setBottom();
}

bool
Vector::isTop() const
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
Vector::setTop()
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setTop();
}

float
Vector::accuracy() const
{
    float maxAccuracy = 0;
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    // Find maximum accuracy and return it.
    for (; it != itend; ++it)
    {
        float accuracy = (*it)->accuracy();
        if (accuracy > maxAccuracy)
            maxAccuracy = accuracy;
    }

    return maxAccuracy;
}

static Vector &
binaryOperation(Vector &result,
                const Domain &a,
                const Domain &b,
                Domain::BinaryOperation operation)
{
    const Vector &aa = checkedCast<Vector>(a),
        &bb = checkedCast<Vector>(b);

    std::vector<Domain*>::iterator it(result.mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb);

    result.collaborate();

    return result;
}

Vector &
Vector::add(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::add);
}

Vector &
Vector::sub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sub);
}

Vector &
Vector::mul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::mul);
}

Vector &
Vector::udiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::udiv);
}

Vector &
Vector::sdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sdiv);
}

Vector &
Vector::urem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::urem);
}

Vector &
Vector::srem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::srem);
}

Vector &
Vector::shl(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::shl);
}

Vector &
Vector::lshr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::lshr);
}

Vector &
Vector::ashr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::ashr);
}

Vector &
Vector::and_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::and_);
}

Vector &
Vector::or_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::or_);
}

Vector &
Vector::xor_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::xor_);
}

Vector &
Vector::icmp(const Domain &a, const Domain &b,
             llvm::CmpInst::Predicate predicate)
{
    const Memory::Pointer
        *aPointer = dynCast<Memory::Pointer>(&a),
        *bPointer = dynCast<Memory::Pointer>(&b);

    if (aPointer && bPointer)
    {
        bool cmpSingle = aPointer->isConstant() && bPointer->isConstant(),
            cmpeq = (*aPointer == *bPointer);

        setBottom();
        switch (predicate)
        {
        case llvm::CmpInst::ICMP_EQ:
        case llvm::CmpInst::ICMP_UGE:
        case llvm::CmpInst::ICMP_ULE:
        case llvm::CmpInst::ICMP_SGE:
        case llvm::CmpInst::ICMP_SLE:
            if (cmpeq && cmpSingle)
            {
                Domain *one = mEnvironment.getConstructors().createInteger(llvm::APInt(1, 1, false));
                join(*one);
                delete one;
            }
            else
            {
                if (predicate == llvm::CmpInst::ICMP_EQ && cmpSingle)
                {
                    Domain *zero = mEnvironment.getConstructors().createInteger(llvm::APInt(1, 0, false));
                    join(*zero);
                    delete zero;
                }
                else
                    setTop();
            }
            break;
        case llvm::CmpInst::ICMP_NE:
            if (cmpSingle)
            {
                llvm::APInt boolean(1, (cmpeq ? 0 : 1), false);
                Domain *result = mEnvironment.getConstructors().createInteger(boolean);
                join(*result);
                delete result;
            }
            else
                setTop();
            break;
        default:
            setTop();
        }

        collaborate();

        return *this;
    }

    const Vector &aa = checkedCast<Vector>(a),
        &bb = checkedCast<Vector>(b);

    std::vector<Domain*>::iterator it(mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != mValues.end(); ++it, ++ita, ++itb)
        (*it)->icmp(**ita, **itb, predicate);

    collaborate();

    return *this;
}

Vector &
Vector::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (*it)->fcmp(a, b, predicate);

    collaborate();

    return *this;
}

static Vector &
castOperation(Vector &result,
              const Domain &value,
              Domain::CastOperation operation)
{
    const Vector &container = checkedCast<Vector>(value);
    std::vector<Domain*>::iterator it = result.mValues.begin();
    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != result.mValues.end(); ++it, ++itc)
        ((**it).*(operation))(**itc);

    result.collaborate();

    return result;
}

Vector &
Vector::trunc(const Domain &value)
{
    return castOperation(*this, value, &Domain::trunc);
}

Vector &
Vector::zext(const Domain &value)
{
    return castOperation(*this, value, &Domain::zext);
}

Vector &
Vector::sext(const Domain &value)
{
    return castOperation(*this, value, &Domain::sext);
}

Vector &
Vector::fptoui(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (**it).fptoui(value);

    collaborate();

    return *this;
}

Vector &
Vector::fptosi(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (**it).fptosi(value);

    collaborate();

    return *this;
}

Domain *
Vector::extractelement(const Domain &index) const
{
    Domain *result = NULL;
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
    {
        Domain *value = (**it).extractelement(index);
        if (result)
        {
            result->meet(*value);
            delete value;
        }
        else
            result = value;
    }

    return result;
}

Vector &
Vector::insertelement(const Domain &array,
                      const Domain &element,
                      const Domain &index)
{
    const Vector &container = checkedCast<Vector>(array);
    CANAL_ASSERT(container.mValues.size() == mValues.size());

    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != itend; ++it, ++itc)
        (**it).insertelement(**itc, element, index);

    collaborate();

    return *this;
}

Vector &
Vector::shufflevector(const Domain &a,
                      const Domain &b,
                      const std::vector<uint32_t> &mask)
{
    const Vector &aa = checkedCast<Vector>(a),
        &bb = checkedCast<Vector>(b);

    CANAL_ASSERT(aa.mValues.size() == mValues.size());
    CANAL_ASSERT(bb.mValues.size() == mValues.size());

    std::vector<Domain*>::iterator it = mValues.begin();
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != mValues.end(); ++it, ++ita, ++itb)
        (**it).shufflevector(**ita, **itb, mask);

    collaborate();

    return *this;
}

Domain *
Vector::extractvalue(const std::vector<unsigned> &indices) const
{
    Domain *result = NULL;
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
    {
        Domain *value = (**it).extractvalue(indices);
        if (result)
        {
            result->meet(*value);
            delete value;
        }
        else
            result = value;
    }

    return result;
}

Vector &
Vector::insertvalue(const Domain &aggregate,
                    const Domain &element,
                    const std::vector<unsigned> &indices)
{
    const Vector &container = checkedCast<Vector>(aggregate);
    CANAL_ASSERT(container.mValues.size() == mValues.size());

    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != itend; ++it, ++itc)
        (**it).insertvalue(**itc, element, indices);

    collaborate();

    return *this;
}

void
Vector::insertvalue(const Domain &element,
                    const std::vector<unsigned> &indices)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (**it).insertvalue(element, indices);

    collaborate();
}

const llvm::Type &
Vector::getValueType() const
{
    const llvm::Type *result = NULL;
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        if (result)
        {
            CANAL_ASSERT(result == &(**it).getValueType());
        }
        else
            result = &(**it).getValueType();
    }

    return *result;
}

Domain *
Vector::loadValue(const llvm::Type &type,
                  const Domain &offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Vector::storeValue(const Domain &value,
                   const Domain &offset,
                   bool isSingleTarget)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Vector::collaborate()
{
    Message inputMessage;

    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        (**it).refine(inputMessage);

        Message outputMessage;
        (**it).extract(outputMessage);

        inputMessage.meet(outputMessage);
    }
}


} // namespace Integer
} // namespace Canal
