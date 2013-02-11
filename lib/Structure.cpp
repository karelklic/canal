#include "Structure.h"
#include "Utils.h"
#include "ProductVector.h"
#include "IntegerSet.h"
#include "IntegerUtils.h"
#include "IntegerInterval.h"
#include "Constructors.h"
#include "Environment.h"

namespace Canal {

Structure::Structure(const Environment &environment,
                     const llvm::StructType &type)
    : Domain(environment, Domain::StructureKind), mType(type)
{
    for (unsigned i = 0; i < type.getNumElements(); ++i)
    {
        const Constructors &c = environment.getConstructors();
        Domain *member = c.create(*type.getElementType(i));
        mMembers.push_back(member);
    }
}

Structure::Structure(const Environment &environment,
                     const llvm::StructType &type,
                     const std::vector<Domain*> &members)
    : Domain(environment, Domain::StructureKind),
      mMembers(members),
      mType(type)
{
}

Structure::Structure(const Structure &value)
    : Domain(value), mMembers(value.mMembers), mType(value.mType)
{
    std::vector<Domain*>::iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
        *it = (*it)->clone();
}

Structure::~Structure()
{
    llvm::DeleteContainerPointers(mMembers);
}

Structure *
Structure::clone() const
{
    return new Structure(*this);
}

size_t
Structure::memoryUsage() const
{
    size_t size = sizeof(Structure);
    size += mMembers.capacity() * sizeof(Domain*);
    std::vector<Domain*>::const_iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
Structure::toString() const
{
    StringStream ss;
    ss << "structure\n";
    std::vector<Domain*>::const_iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
        ss << indent((*it)->toString(), 4);

    return ss.str();
}

void
Structure::setZero(const llvm::Value *place)
{
    std::vector<Domain*>::iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
        (*it)->setZero(place);
}

typedef bool(Domain::*CmpOperation)(const Domain&)const;

static bool
compareMembers(const Structure &a, const Domain &b, CmpOperation operation)
{
    const Structure &bb = checkedCast<Structure>(b);
    if (a.mMembers.size() != bb.mMembers.size())
        return false;

    std::vector<Domain*>::const_iterator ita = a.mMembers.begin(),
        itaend = a.mMembers.end(),
        itb = bb.mMembers.begin();

    for (; ita != itaend; ++ita, ++itb)
    {
        if (!((**ita).*(operation))(**itb))
            return false;
    }

    return true;
}

bool
Structure::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    return compareMembers(*this, value, &Domain::operator==);
}

bool
Structure::operator<(const Domain &value) const
{
    if (this == &value)
        return false;

    return compareMembers(*this, value, &Domain::operator<);
}

typedef Domain&(Domain::*JoinOrMeetOperation)(const Domain&);

static Structure &
joinOrMeet(Structure &a, const Domain &b, JoinOrMeetOperation op)
{
    const Structure &bb = checkedCast<Structure>(b);
    CANAL_ASSERT(a.mMembers.size() == bb.mMembers.size());
    std::vector<Domain*>::iterator ita = a.mMembers.begin(),
        itaend = a.mMembers.end();

    std::vector<Domain*>::const_iterator itb = bb.mMembers.begin();
    for (; ita != itaend; ++ita, ++itb)
        ((*ita)->*(op))(**itb);

    return a;
}

Structure &
Structure::join(const Domain &value)
{
    return joinOrMeet(*this, value, &Domain::join);
}

Structure &
Structure::meet(const Domain &value)
{
    return joinOrMeet(*this, value, &Domain::meet);
}

bool
Structure::isBottom() const
{
    std::vector<Domain*>::const_iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
    {
        if (!(*it)->isBottom())
            return false;
    }

    return true;
}

void
Structure::setBottom()
{
    std::vector<Domain*>::iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
        (*it)->setBottom();
}

bool
Structure::isTop() const
{
    std::vector<Domain*>::const_iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
    {
        if (!(*it)->isTop())
            return false;
    }

    return true;
}

void
Structure::setTop()
{
    std::vector<Domain*>::iterator it = mMembers.begin(),
        itend = mMembers.end();

    for (; it != itend; ++it)
        (*it)->setTop();
}

float
Structure::accuracy() const
{
    std::vector<Domain*>::const_iterator it = mMembers.begin(),
        itend = mMembers.end();

    float result = 0;
    for (; it != itend; ++it)
        result += (*it)->accuracy();

    return result / mMembers.size();
}

Domain *
Structure::extractelement(const Domain &index) const
{
    // Replace a single element or set to top.
    const Integer::Set &set = Integer::Utils::getSet(index);
    CANAL_ASSERT(!set.isTop() && set.mValues.size() == 1);
    CANAL_ASSERT(set.mValues.begin()->getBitWidth() <= 64);
    uint64_t numOffset = set.mValues.begin()->getZExtValue();
    return mMembers[numOffset]->clone();
}

Structure &
Structure::insertelement(const Domain &array,
                         const Domain &element,
                         const Domain &index)
{
    const Structure &structure = checkedCast<Structure>(array);
    CANAL_ASSERT(&mType == &structure.mType);
    CANAL_ASSERT(mMembers.size() == structure.mMembers.size());

    // Replace a single element or set to top.
    const Integer::Set &set = Integer::Utils::getSet(index);
    if (set.isTop() || set.mValues.size() != 1)
        setTop();
    else
    {
        // Copy the original values.
        std::vector<Domain*>::const_iterator itA = mMembers.begin(),
            itAend = mMembers.end(),
            itB = structure.mMembers.begin();

        for (; itA != itAend; ++itA, ++itB)
            (*itA)->join(**itB);

        // Set the single element.
        CANAL_ASSERT(set.mValues.begin()->getBitWidth() <= 64);
        uint64_t numOffset = set.mValues.begin()->getZExtValue();

        delete mMembers[numOffset];
        mMembers[numOffset] = element.clone();
    }

    return *this;
}

Domain *
Structure::extractvalue(const std::vector<unsigned> &indices) const
{
    CANAL_ASSERT(!indices.empty());
    unsigned index = indices[0];
    CANAL_ASSERT(index < mMembers.size());
    if (indices.size() > 1)
    {
        return mMembers[index]->extractvalue(std::vector<unsigned>(indices.begin() + 1,
                                                                   indices.end()));
    }
    else
        return mMembers[index]->clone();
}

Structure &
Structure::insertvalue(const Domain &aggregate,
                       const Domain &element,
                       const std::vector<unsigned> &indices)
{
    const Structure &structure = checkedCast<Structure>(aggregate);
    CANAL_ASSERT(&mType == &structure.mType);
    CANAL_ASSERT(mMembers.size() == structure.mMembers.size());

    // Copy the original values.
    std::vector<Domain*>::const_iterator itA = mMembers.begin(),
        itAend = mMembers.end(),
        itB = structure.mMembers.begin();

    for (; itA != itAend; ++itA, ++itB)
        (*itA)->join(**itB);

    // Insert the element.
    insertvalue(element, indices);
    return *this;
}

void
Structure::insertvalue(const Domain &element,
                       const std::vector<unsigned> &indices)
{
    CANAL_ASSERT(!indices.empty());
    unsigned index = indices[0];
    CANAL_ASSERT(index < mMembers.size());
    if (indices.size() > 1)
    {
        mMembers[index]->insertvalue(element,
                                     std::vector<unsigned>(indices.begin() + 1,
                                                           indices.end()));
    }
    else
    {
        delete mMembers[index];
        mMembers[index] = element.clone();
    }
}

Domain *
Structure::loadValue(const llvm::Type &type,
                     const Domain &offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Structure::storeValue(const Domain &value,
                      const Domain &offset,
                      bool isSingleTarget)
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
