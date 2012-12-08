#include "Structure.h"
#include "Utils.h"
#include "IntegerContainer.h"
#include "IntegerSet.h"
#include "IntegerUtils.h"
#include "IntegerInterval.h"

namespace Canal {

Structure::Structure(const Environment &environment,
                     const std::vector<Domain*> &members)
    : Domain(environment), mMembers(members)
{

}

Structure::Structure(const Structure &value)
    : Domain(value), mMembers(value.mMembers)
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

bool
Structure::operator>(const Domain &value) const
{
    if (this == &value)
        return false;

    return compareMembers(*this, value, &Domain::operator>);
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

std::vector<Domain*>
Structure::getItem(const Domain &offset) const
{
    std::vector<Domain*> result;

    // First try an enumeration, then interval.
    const Integer::Set &set = Integer::Utils::getSet(offset);
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
            if (numOffset >= mMembers.size())
                continue;

            result.push_back(mMembers[numOffset]);
        }

        // At least one of the offsets in the set should point
        // to the array.  Otherwise it might be a bug in the
        // interpreter that requires investigation.
        CANAL_ASSERT(!result.empty());
        return result;
    }

    const Integer::Interval &interval = Integer::Utils::getInterval(offset);
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
        CANAL_ASSERT(from < mMembers.size());
        if (to >= mMembers.size())
            to = mMembers.size();

        result.insert(result.end(),
                      mMembers.begin() + from,
                      mMembers.begin() + to);

        return result;
    }

    // Both set and interval are set to the top value, so return
    // all members.
    result.insert(result.end(), mMembers.begin(), mMembers.end());

    // Zero length arrays are not supported.
    CANAL_ASSERT(!result.empty());
    return result;
}

Domain *
Structure::getItem(uint64_t offset) const
{
    CANAL_ASSERT_MSG(offset < mMembers.size(),
                     "Offset out of bounds.");

    return mMembers[offset];
}

void
Structure::setItem(const Domain &offset, const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Structure::setItem(uint64_t offset, const Domain &value)
{
    CANAL_ASSERT_MSG(offset < mMembers.size(),
                     "Offset out of bounds.");

    mMembers[offset]->join(value);
}

} // namespace Canal
