#include "IntegerContainer.h"
#include "IntegerBits.h"
#include "IntegerEnumeration.h"
#include "IntegerRange.h"
#include "Constant.h"
#include "Utils.h"
#include "APIntUtils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Container::Container(unsigned numBits)
{
    mValues.push_back(new Bits(numBits));
    mValues.push_back(new Enumeration(numBits));
    mValues.push_back(new Range(numBits));
}

Container::Container(const llvm::APInt &number)
{
    mValues.push_back(new Bits(number));
    mValues.push_back(new Enumeration(number));
    mValues.push_back(new Range(number));
}

Container::Container(const Container &container)
{
    mValues = container.mValues;
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        *it = static_cast<Value*>((*it)->clone());
}

Container::~Container()
{
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        delete *it;
}

int
Container::getBitWidth() const
{
    return getEnumeration().mNumBits;
}

Bits &
Container::getBits()
{
    return dynCast<Bits&>(*mValues[0]);
}

const Bits &
Container::getBits() const
{
    return dynCast<const Bits&>(*mValues[0]);
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

Range &
Container::getRange()
{
    return dynCast<Range&>(*mValues[2]);
}

const Range &
Container::getRange() const
{
    return dynCast<const Range&>(*mValues[2]);
}

bool
Container::signedMin(llvm::APInt &result) const
{
    if (!getEnumeration().signedMin(result))
        return false;

    llvm::APInt temp(getBitWidth(), 0);
    if (!getRange().signedMin(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    if (!getBits().signedMin(temp))
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
    if (!getRange().signedMax(temp))
        return false;

    if (result.slt(temp))
        result = temp;

    if (!getBits().signedMax(temp))
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
    if (!getRange().unsignedMin(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    if (!getBits().unsignedMin(temp))
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
    if (!getRange().unsignedMax(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    if (!getBits().unsignedMax(temp))
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

bool
Container::operator==(const Value &value) const
{
    const Container *container = dynCast<const Container*>(&value);
    if (!container)
        return false;

    CANAL_ASSERT(mValues.size() == container->mValues.size());
    std::vector<Value*>::const_iterator ita(mValues.begin()),
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
    std::vector<Value*>::iterator it = mValues.begin();
    if (const Constant *constant = dynCast<const Constant*>(&value))
    {
        for (; it != mValues.end(); ++it)
            (*it)->merge(*constant);
        return;
    }

    const Container &container = dynCast<const Container&>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Value*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->merge(**it2);
}

size_t
Container::memoryUsage() const
{
    size_t size(0);
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        size += (*it)->memoryUsage();
    return size;
}

std::string
Container::toString() const
{
    std::stringstream ss;
    ss << "integerContainer" << std::endl;
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
        ss << indent((*it)->toString(), 4);
    return ss.str();
}

// Converts value to container.  If the value is a constant, it is
// converted to container and deleteAfter is set to true.
static const Container *
asContainer(const Value &value, bool &deleteAfter)
{
    deleteAfter = false;
    const Container *container = dynCast<const Container*>(&value);
    if (!container)
    {
        const Constant *constant = dynCast<const Constant*>(&value);
        CANAL_ASSERT_MSG(constant,
                         "Unsupported type cannot be converted to integer container.");
        container = new Container(constant->getAPInt());
        deleteAfter = true;
    }

    return container;
}

static void
applyBinaryOperation(Container &result,
                     const Value &a,
                     const Value &b,
                     void(Value::*operation)(const Value&, const Value&))
{
    bool deleteAA, deleteBB;
    const Container *aa = asContainer(a, deleteAA),
        *bb = asContainer(b, deleteBB);

    std::vector<Value*>::iterator it(result.mValues.begin());
    std::vector<Value*>::const_iterator ita = aa->mValues.begin(),
        itb = bb->mValues.begin();

    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb);

    if (deleteAA)
        delete aa;
    if (deleteBB)
        delete bb;
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

static void
applyCmpOperation(Container &result,
                  const Value &a,
                  const Value &b,
                  llvm::CmpInst::Predicate predicate,
                  void(Value::*operation)(const Value&, const Value&,
                                          llvm::CmpInst::Predicate))
{
    bool deleteAA, deleteBB;
    const Container *aa = asContainer(a, deleteAA),
        *bb = asContainer(b, deleteBB);

    std::vector<Value*>::iterator it(result.mValues.begin());
    std::vector<Value*>::const_iterator ita = aa->mValues.begin(),
        itb = bb->mValues.begin();

    for (; it != result.mValues.end(); ++it, ++ita, ++itb)
        ((**it).*(operation))(**ita, **itb, predicate);

    if (deleteAA)
        delete aa;
    if (deleteBB)
        delete bb;
}


void
Container::icmp(const Value &a, const Value &b,
                llvm::CmpInst::Predicate predicate)
{
    applyCmpOperation(*this, a, b, predicate, &Value::icmp);
}

void
Container::fcmp(const Value &a, const Value &b,
                llvm::CmpInst::Predicate predicate)
{
    applyCmpOperation(*this, a, b, predicate, &Value::fcmp);
}

float Container::accuracy() const
{
    float accuracy = 0;
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyValue *accuracyValue =
            dynCast<const AccuracyValue*>(*it);

        if (!accuracyValue)
            continue;

        float localAccuracy = accuracyValue->accuracy();
        if (localAccuracy > accuracy)
            accuracy = localAccuracy;
    }
    return accuracy;
}

bool Container::isBottom() const
{
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyValue *accuracyValue =
            dynCast<const AccuracyValue*>(*it);

        if (!accuracyValue)
            continue;

        if (!accuracyValue->isBottom())
            return false;
    }
    return true;
}

void Container::setBottom()
{
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        AccuracyValue *accuracyValue =
            dynCast<AccuracyValue*>(*it);

        if (!accuracyValue)
            continue;

        accuracyValue->setBottom();
    }
}

bool Container::isTop() const
{
    std::vector<Value*>::const_iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        const AccuracyValue *accuracyValue =
            dynCast<const AccuracyValue*>(*it);

        if (!accuracyValue)
            continue;

        if (!accuracyValue->isTop())
            return false;
    }
    return true;
}

void Container::setTop()
{
    std::vector<Value*>::iterator it = mValues.begin();
    for (; it != mValues.end(); ++it)
    {
        AccuracyValue *accuracyValue = dynCast<AccuracyValue*>(*it);
        if (!accuracyValue)
            continue;

        accuracyValue->setTop();
    }
}

} // namespace Integer
} // namespace Canal
