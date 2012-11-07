#include "IntegerContainer.h"
#include "IntegerBitfield.h"
#include "IntegerEnumeration.h"
#include "IntegerInterval.h"
#include "Utils.h"
#include "StringUtils.h"
#include "APIntUtils.h"
#include "Pointer.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Container::Container(const Environment &environment,
                     unsigned bitWidth)
    : Domain(environment)
{
    mValues.push_back(new Bitfield(environment, bitWidth));
    mValues.push_back(new Enumeration(environment, bitWidth));
    mValues.push_back(new Interval(environment, bitWidth));
}

Container::Container(const Environment &environment,
                     const llvm::APInt &number)
    : Domain(environment)
{
    mValues.push_back(new Bitfield(environment, number));
    mValues.push_back(new Enumeration(environment, number));
    mValues.push_back(new Interval(environment, number));
}

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
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete *it;
}

unsigned
Container::getBitWidth() const
{
    return getEnumeration().getBitWidth();
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

Enumeration &
Container::getEnumeration()
{
    return dynCast<Enumeration&>(*mValues[1]);
}

const Enumeration &
Container::getEnumeration() const
{
    return dynCast<const Enumeration&>(*mValues[1]);
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
    if (!getEnumeration().signedMin(result))
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
    if (!getEnumeration().signedMax(result))
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
    if (!getEnumeration().unsignedMin(result))
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
    if (!getEnumeration().unsignedMax(result))
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

Container *
Container::clone() const
{
    return new Container(*this);
}

Container *
Container::cloneCleaned() const
{
    return new Container(mEnvironment, getBitWidth());
}

bool
Container::operator==(const Domain &value) const
{
    const Container *container = dynCast<const Container*>(&value);
    if (!container)
        return false;

    CANAL_ASSERT(mValues.size() == container->mValues.size());
    std::vector<Domain*>::const_iterator ita(mValues.begin()),
        itb(container->mValues.begin());
    for (; ita != mValues.end(); ++ita, ++itb)
    {
        if (**ita != **itb)
            return false;
    }

    return true;
}

void
Container::merge(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    const Container &container = dynCast<const Container&>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->merge(**it2);
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
    std::stringstream ss;
    ss << "integerContainer" << std::endl;
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << indent((*it)->toString(), 4);
    return ss.str();
}

static void
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
}

void
Container::add(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::add);
}

void
Container::sub(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::sub);
}

void
Container::mul(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::mul);
}

void
Container::udiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::udiv);
}

void
Container::sdiv(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::sdiv);
}

void
Container::urem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::urem);
}

void
Container::srem(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::srem);
}

void
Container::shl(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::shl);
}

void
Container::lshr(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::lshr);
}

void
Container::ashr(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::ashr);
}

void
Container::and_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::and_);
}

void
Container::or_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::or_);
}

void
Container::xor_(const Domain &a, const Domain &b)
{
    binaryOperation(*this, a, b, &Domain::xor_);
}

void
Container::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    const Canal::Pointer::InclusionBased *aPointer =
            dynCast<const Canal::Pointer::InclusionBased*>(&a),
        *bPointer = dynCast<const Canal::Pointer::InclusionBased*>(&b);

    if (aPointer && bPointer)
    {
        bool cmpSingle = aPointer->isSingleTarget() && bPointer->isSingleTarget(),
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
                merge(Container(mEnvironment,
                                llvm::APInt(1, 1, false)));
            else
            {
                if (predicate == llvm::CmpInst::ICMP_EQ && cmpSingle)
                {
                    merge(Container(mEnvironment,
                                    llvm::APInt(1, 0, false)));
                }
                else
                    setTop();
            }
            break;
        case llvm::CmpInst::ICMP_NE:
            if (cmpSingle)
            {
                merge(Container(mEnvironment,
                                llvm::APInt(1, (cmpeq ? 0 : 1), false)));
            }
            else
                setTop();
            break;
        default:
            setTop();
        }
        return;
    }

    const Container &aa = dynCast<const Container&>(a),
        &bb = dynCast<const Container&>(b);

    std::vector<Domain*>::iterator it(mValues.begin());
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != mValues.end(); ++it, ++ita, ++itb)
        (*it)->icmp(**ita, **itb, predicate);
}

void
Container::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (*it)->fcmp(a, b, predicate);
}

static void
castOperation(Container &result,
              const Domain &value,
              Domain::CastOperation operation)
{
    const Container &container = dynCast<const Container&>(value);
    std::vector<Domain*>::iterator it = result.mValues.begin();
    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != result.mValues.end(); ++it, ++itc)
        ((**it).*(operation))(**itc);
}

void
Container::trunc(const Domain &value)
{
    castOperation(*this, value, &Domain::trunc);
}

void
Container::zext(const Domain &value)
{
    castOperation(*this, value, &Domain::zext);
}

void
Container::sext(const Domain &value)
{
    castOperation(*this, value, &Domain::sext);
}

void
Container::fptoui(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (**it).fptoui(value);
}

void
Container::fptosi(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (**it).fptosi(value);
}

float
Container::accuracy() const
{
    float accuracy = 0;
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyDomain *accuracyValue =
            dynCast<const AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        float localAccuracy = accuracyValue->accuracy();
        if (localAccuracy > accuracy)
            accuracy = localAccuracy;
    }
    return accuracy;
}

bool
Container::isBottom() const
{
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyDomain *accuracyValue =
            dynCast<const AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        if (!accuracyValue->isBottom())
            return false;
    }
    return true;
}

void
Container::setBottom()
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        AccuracyDomain *accuracyValue =
            dynCast<AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        accuracyValue->setBottom();
    }
}

bool
Container::isTop() const
{
    std::vector<Domain*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyDomain *accuracyValue =
            dynCast<const AccuracyDomain*>(*it);

        if (!accuracyValue)
            continue;

        if (!accuracyValue->isTop())
            return false;
    }
    return true;
}

void
Container::setTop()
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        AccuracyDomain *accuracyValue = dynCast<AccuracyDomain*>(*it);
        if (!accuracyValue)
            continue;

        accuracyValue->setTop();
    }
}

bool
Container::isSingleValue() const
{
    return getBitfield().isSingleValue()
        && getEnumeration().isSingleValue()
        && getInterval().isSingleValue();
}

void
Container::setZero(const llvm::Value *place)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (*it)->setZero(place);
}

} // namespace Integer
} // namespace Canal
