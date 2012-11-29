#include "IntegerContainer.h"
#include "IntegerBitfield.h"
#include "IntegerSet.h"
#include "IntegerInterval.h"
#include "Environment.h"
#include "Constructors.h"
#include "Utils.h"
#include "APIntUtils.h"
#include "Pointer.h"

namespace Canal {
namespace Integer {

Container::Container(const Environment &environment)
    : Domain(environment) {}

Container::Container(const Container &value)
    : Domain(value)
{
    mValues = value.mValues;
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = (*it)->clone();
}

Container::~Container()
{
    llvm::DeleteContainerPointers(mValues);
}

unsigned
Container::getBitWidth() const
{
    return getSet().getBitWidth();
}

Bitfield &
Container::getBitfield()
{
    return dynCast<Bitfield&>(*mValues[0]);
}

const Bitfield &
Container::getBitfield() const
{
    return dynCast<const Bitfield&>(*mValues[0]);
}

Set &
Container::getSet()
{
    return dynCast<Set&>(*mValues[1]);
}

const Set &
Container::getSet() const
{
    return dynCast<const Set&>(*mValues[1]);
}

Interval &
Container::getInterval()
{
    return dynCast<Interval&>(*mValues[2]);
}

const Interval &
Container::getInterval() const
{
    return dynCast<const Interval&>(*mValues[2]);
}

bool
Container::signedMin(llvm::APInt &result) const
{
    if (!getSet().signedMin(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getInterval().signedMin(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    if (!getBitfield().signedMin(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    return true;
}

bool
Container::signedMax(llvm::APInt &result) const
{
    if (!getSet().signedMax(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getInterval().signedMax(temp))
        return false;

    if (result.slt(temp))
        result = temp;

    if (!getBitfield().signedMax(temp))
        return false;

    if (result.slt(temp))
        result = temp;

    return true;
}

bool
Container::unsignedMin(llvm::APInt &result) const
{
    if (!getSet().unsignedMin(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getInterval().unsignedMin(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    if (!getBitfield().unsignedMin(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    return true;
}

bool
Container::unsignedMax(llvm::APInt &result) const
{
    if (!getSet().unsignedMax(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getInterval().unsignedMax(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    if (!getBitfield().unsignedMax(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    return true;
}

bool
Container::isConstant() const
{
    return getBitfield().isConstant()
        && getSet().isConstant()
        && getInterval().isConstant();
}

Container *
Container::clone() const
{
    return new Container(*this);
}

size_t
Container::memoryUsage() const
{
    size_t size(0);
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();
    return size;
}

std::string
Container::toString() const
{
    StringStream ss;
    ss << "integerContainer\n";
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        ss << indent((*it)->toString(), 4);

    return ss.str();
}

void
Container::setZero(const llvm::Value *place)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setZero(place);
}

bool
Container::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const Container *container = dynCast<const Container*>(&value);
    if (!container)
        return false;

    CANAL_ASSERT(mValues.size() == container->mValues.size());
    std::vector<Domain*>::const_iterator ita(mValues.begin()),
        itb(container->mValues.begin());

    for (; ita != mValues.end(); ++ita, ++itb)
    {
        // If iterators point to the same object skip casting and
        // comparison of object.
        if (*ita == *itb)
            continue;

        if (**ita != **itb)
            return false;
    }

    return true;
}

bool
Container::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Container::operator>(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

Container &
Container::join(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    const Container &container = dynCast<const Container&>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->join(**it2);

    return *this;
}

Container &
Container::meet(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Container::isBottom() const
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
Container::setBottom()
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setBottom();
}

bool
Container::isTop() const
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
Container::setTop()
{
    std::vector<Domain*>::const_iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (*it)->setTop();
}

float
Container::accuracy() const
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

static Container &
binaryOperation(Container &result,
                const Domain &a,
                const Domain &b,
                Domain::BinaryOperation operation)
{
    const Container &aa = dynCast<const Container&>(a),
        &bb = dynCast<const Container&>(b);

    std::vector<Domain*>::iterator it(result.mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb);

    return result;
}

Container &
Container::add(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::add);
}

Container &
Container::sub(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sub);
}

Container &
Container::mul(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::mul);
}

Container &
Container::udiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::udiv);
}

Container &
Container::sdiv(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::sdiv);
}

Container &
Container::urem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::urem);
}

Container &
Container::srem(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::srem);
}

Container &
Container::shl(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::shl);
}

Container &
Container::lshr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::lshr);
}

Container &
Container::ashr(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::ashr);
}

Container &
Container::and_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::and_);
}

Container &
Container::or_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::or_);
}

Container &
Container::xor_(const Domain &a, const Domain &b)
{
    return binaryOperation(*this, a, b, &Domain::xor_);
}

Container &
Container::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    const Pointer::Pointer
        *aPointer = dynCast<const Pointer::Pointer*>(&a),
        *bPointer = dynCast<const Pointer::Pointer*>(&b);

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

        return *this;
    }

    const Container &aa = dynCast<const Container&>(a),
        &bb = dynCast<const Container&>(b);

    std::vector<Domain*>::iterator it(mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != mValues.end(); ++it, ++ita, ++itb)
        (*it)->icmp(**ita, **itb, predicate);

    return *this;
}

Container &
Container::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (*it)->fcmp(a, b, predicate);

    return *this;
}

static Container &
castOperation(Container &result,
              const Domain &value,
              Domain::CastOperation operation)
{
    const Container &container = dynCast<const Container&>(value);
    std::vector<Domain*>::iterator it = result.mValues.begin();
    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != result.mValues.end(); ++it, ++itc)
        ((**it).*(operation))(**itc);

    return result;
}

Container &
Container::trunc(const Domain &value)
{
    return castOperation(*this, value, &Domain::trunc);
}

Container &
Container::zext(const Domain &value)
{
    return castOperation(*this, value, &Domain::zext);
}

Container &
Container::sext(const Domain &value)
{
    return castOperation(*this, value, &Domain::sext);
}

Container &
Container::fptoui(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (**it).fptoui(value);

    return *this;
}

Container &
Container::fptosi(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (**it).fptosi(value);

    return *this;
}

} // namespace Integer
} // namespace Canal
