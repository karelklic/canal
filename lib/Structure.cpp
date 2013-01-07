#include "Structure.h"
#include "Utils.h"
#include "IntegerContainer.h"
#include "IntegerSet.h"
#include "IntegerUtils.h"
#include "IntegerInterval.h"
#include "Environment.h"
#include "Constructors.h"
#include "Memory.h"

namespace Canal {

Structure::Structure(const Environment &environment,
                     const llvm::StructType &type)
    : Domain(environment), mType(type)
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
    : Domain(environment), mMembers(members), mType(type)
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
    const Structure *bb = dynCast<const Structure*>(&b);
    if (!bb)
        return false;

    if (a.mMembers.size() != bb->mMembers.size())
        return false;

    std::vector<Domain*>::const_iterator ita = a.mMembers.begin(),
        itaend = a.mMembers.end(),
        itb = bb->mMembers.begin();

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
    const Structure &bb = dynCast<const Structure&>(b);
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

Structure &
Structure::insertelement(const Domain &array,
                         const Domain &element,
                         const Domain &index)
{
    const Structure &s = dynCast<const Structure&>(array);

    CANAL_ASSERT(&s.mType == &mType &&
                 s.mMembers.size() == mMembers.size());

    std::vector<Domain*>::const_iterator
        it = mMembers.begin(),
        itend = mMembers.end(),
        its = s.mMembers.begin();

    CANAL_NOT_IMPLEMENTED();
}

const llvm::StructType &
Structure::getValueType() const
{
    return mType;
}

Domain *
Structure::getValueCell(uint64_t offset) const
{
    Domain *cell = mEnvironment.getConstructors().createInteger(8);
    cell->setTop();
    return cell;
}

void
Structure::mergeValueCell(uint64_t offset, const Domain &value)
{
    setTop();
}

} // namespace Canal
