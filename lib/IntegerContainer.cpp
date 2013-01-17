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
    : Domain(environment, Domain::IntegerContainerKind)
{
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
    llvm::DeleteContainerPointers(mValues);
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

    const Container &container = checkedCast<Container>(value);
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
Container::operator<(const Domain &value) const
{
    if (this == &value)
        return false;

    const Container &container = checkedCast<Container>(value);
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

Container &
Container::join(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    const Container &container = checkedCast<Container>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->join(**it2);

    return *this;
}

Container &
Container::meet(const Domain &value)
{
    std::vector<Domain*>::iterator it = mValues.begin();
    const Container &container = checkedCast<Container>(value);
    CANAL_ASSERT(mValues.size() == container.mValues.size());
    std::vector<Domain*>::const_iterator it2 = container.mValues.begin();
    for (; it != mValues.end(); ++it, ++it2)
        (*it)->meet(**it2);

    return *this;
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
    const Container &aa = checkedCast<Container>(a),
        &bb = checkedCast<Container>(b);

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
        *aPointer = dynCast<Pointer::Pointer>(&a),
        *bPointer = dynCast<Pointer::Pointer>(&b);

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

    const Container &aa = checkedCast<Container>(a),
        &bb = checkedCast<Container>(b);

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
    const Container &container = checkedCast<Container>(value);
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

Domain *
Container::extractelement(const Domain &index) const
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

Container &
Container::insertelement(const Domain &array,
                         const Domain &element,
                         const Domain &index)
{
    const Container &container = checkedCast<Container>(array);
    CANAL_ASSERT(container.mValues.size() == mValues.size());

    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != itend; ++it, ++itc)
        (**it).insertelement(**itc, element, index);

    return *this;
}

Container &
Container::shufflevector(const Domain &a,
                         const Domain &b,
                         const std::vector<uint32_t> &mask)
{
    const Container &aa = checkedCast<Container>(a),
        &bb = checkedCast<Container>(b);

    CANAL_ASSERT(aa.mValues.size() == mValues.size());
    CANAL_ASSERT(bb.mValues.size() == mValues.size());

    std::vector<Domain*>::iterator it = mValues.begin();
    std::vector<Domain*>::const_iterator ita = aa.mValues.begin(),
        itb = bb.mValues.begin();

    for (; it != mValues.end(); ++it, ++ita, ++itb)
        (**it).shufflevector(**ita, **itb, mask);

    return *this;
}

Domain *
Container::extractvalue(const std::vector<unsigned> &indices) const
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

Container &
Container::insertvalue(const Domain &aggregate,
                       const Domain &element,
                       const std::vector<unsigned> &indices)
{
    const Container &container = checkedCast<Container>(aggregate);
    CANAL_ASSERT(container.mValues.size() == mValues.size());

    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    std::vector<Domain*>::const_iterator itc = container.mValues.begin();
    for (; it != itend; ++it, ++itc)
        (**it).insertvalue(**itc, element, indices);

    return *this;
}

void
Container::insertvalue(const Domain &element,
                       const std::vector<unsigned> &indices)
{
    std::vector<Domain*>::iterator it = mValues.begin(),
        itend = mValues.end();

    for (; it != itend; ++it)
        (**it).insertvalue(element, indices);
}

Domain *
Container::load(const llvm::Type &type,
                const std::vector<Domain*> &offsets) const
{
    if (offsets.empty())
    {
        if (&type == &getValueType())
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

Container &
Container::store(const Domain &value,
                 const std::vector<Domain*> &offsets,
                 bool overwrite)
{
    if (offsets.empty())
    {
        const Container &container = checkedCast<Container>(value);
        CANAL_ASSERT(container.mValues.size() == mValues.size());

        std::vector<Domain*>::iterator it = mValues.begin(),
            itend = mValues.end();

        std::vector<Domain*>::const_iterator itc = container.mValues.begin();
        for (; it != itend; ++it, ++itc)
            (**it).store(**itc, offsets, overwrite);
    }
    else
    {
        std::vector<Domain*>::iterator it = mValues.begin(),
            itend = mValues.end();

        for (; it != itend; ++it)
            (**it).store(value, offsets, overwrite);
    }

    return *this;
}

const llvm::Type &
Container::getValueType() const
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

} // namespace Integer
} // namespace Canal
