#include "Integer.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Container::Container(unsigned numBits)
{
    mValues.push_back(new Bits(numBits));
}

Container::Container(const llvm::APInt &number)
{
    mValues.push_back(new Bits(number));
}

Container::Container(const Container &container)
{
    mValues = container.mValues;
    std::vector<AccuracyValue*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = static_cast<AccuracyValue*>((*it)->clone());
}

Container::~Container()
{
    std::vector<AccuracyValue*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete *it;
}

Container *
Container::clone() const
{
    return new Container(*this);
}

bool
Container::operator==(const Value &value) const
{
    const Container *container = dynamic_cast<const Container*>(&value);
    if (!container)
        return false;

    CANAL_ASSERT(mValues.size() == container->mValues.size());
    std::vector<AccuracyValue*>::const_iterator ita(mValues.begin()),
        itb(container->mValues.begin());
    for (; ita != mValues.end(); ++ita, ++itb)
    {
        if (**ita != **itb)
            return false;
    }

    return true;
}

void
Container::merge(const Value &value)
{
    std::vector<AccuracyValue*>::iterator it = mValues.begin();
    if (const Constant *constant = dynamic_cast<const Constant*>(&value))
    {
        for (; it != mValues.end(); ++it)
            (*it)->merge(*constant);
        return;
    }

    const Container &container = dynamic_cast<const Container&>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<AccuracyValue*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->merge(**it2);
}

size_t
Container::memoryUsage() const
{
    size_t size(0);
    std::vector<AccuracyValue*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();
    return size;
}

std::string
Container::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Integer::Container: [" << std::endl;
    std::vector<AccuracyValue*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << "    " << indentExceptFirstLine((*it)->toString(state), 4) << std::endl;
    ss << "]";
    return ss.str();
}


static void
applyBinaryOperation(Container &result, const Value &a, const Value &b, void(Value::*operation)(const Value&, const Value&))
{
    const Canal::Constant *aconstant = dynamic_cast<const Canal::Constant*>(&a);
    const Container *acontainer = dynamic_cast<const Container*>(&a);
    CANAL_ASSERT(aconstant || acontainer);
    CANAL_ASSERT(!acontainer || result.mValues.size() == acontainer->mValues.size());

    const Canal::Constant *bconstant = dynamic_cast<const Canal::Constant*>(&b);
    const Container *bcontainer = dynamic_cast<const Container*>(&b);
    CANAL_ASSERT(bconstant || bcontainer);
    CANAL_ASSERT(!bcontainer || result.mValues.size() == bcontainer->mValues.size());

    std::vector<AccuracyValue*>::iterator it(result.mValues.begin());
    std::vector<AccuracyValue*>::const_iterator ita(acontainer ? acontainer->mValues.begin() : it);
    std::vector<AccuracyValue*>::const_iterator itb(bcontainer ? bcontainer->mValues.begin() : it);
    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
    {
        const Value &avalue = acontainer ? static_cast<const Value&>(**ita) : *aconstant;
        const Value &bvalue = bcontainer ? static_cast<const Value&>(**itb) : *bconstant;
        ((**it).*(operation))(avalue, bvalue);
    }
}

void
Container::add(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::add);
}

void
Container::sub(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::sub);
}

void
Container::mul(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::mul);
}

void
Container::udiv(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::udiv);
}

void
Container::sdiv(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::sdiv);
}

void
Container::urem(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::urem);
}

void
Container::srem(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::srem);
}

void
Container::shl(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::shl);
}

void
Container::lshr(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::lshr);
}

void
Container::ashr(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::ashr);
}

void
Container::and_(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::and_);
}

void
Container::or_(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::or_);
}

void
Container::xor_(const Value &a, const Value &b)
{
    applyBinaryOperation(*this, a, b, &Value::xor_);
}

float Container::accuracy() const
{
    float accuracy = 0;
    std::vector<AccuracyValue*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        float localAccuracy = (*it)->accuracy();
        if (localAccuracy > accuracy)
            accuracy = localAccuracy;
    }
    return accuracy;
}

bool Container::isBottom() const
{
    std::vector<AccuracyValue*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        if (!(*it)->isBottom())
            return false;
    }
    return true;
}

void Container::setBottom()
{
    std::vector<AccuracyValue*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (*it)->setBottom();
}

bool Container::isTop() const
{
    std::vector<AccuracyValue*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        if (!(*it)->isTop())
            return false;
    }
    return true;
}

void Container::setTop()
{
    std::vector<AccuracyValue*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        (*it)->setTop();
}


Bits &Container::getBits()
{
    return dynamic_cast<Bits&>(*mValues[0]);
}

const Bits &Container::getBits() const
{
    return dynamic_cast<Bits&>(*mValues[0]);
}

} // namespace Integer
} // namespace Canal
