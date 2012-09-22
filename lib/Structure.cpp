#include "Structure.h"
#include "Utils.h"
#include "IntegerContainer.h"
#include "IntegerEnumeration.h"
#include "IntegerInterval.h"
#include <sstream>

namespace Canal {

Structure::Structure(const Environment &environment)
    : Domain(environment)
{
}

Structure::Structure(const Structure &structure)
    : Domain(structure.mEnvironment)
{
    mMembers = structure.mMembers;
    std::vector<Domain*>::iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        *it = (*it)->clone();
}

Structure::~Structure()
{
    std::vector<Domain*>::iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        delete (*it);
}

Structure *
Structure::clone() const
{
    return new Structure(*this);
}

Structure *
Structure::cloneCleaned() const
{
    return new Structure(mEnvironment);
}

bool
Structure::operator==(const Domain &value) const
{
    const Structure *structure = dynCast<const Structure*>(&value);
    if (!structure)
        return false;

    if (mMembers.size() != structure->mMembers.size())
        return false;

    std::vector<Domain*>::const_iterator itA = mMembers.begin(),
        itAend = mMembers.end(),
        itB = structure->mMembers.begin();
    for (; itA != itAend; ++itA, ++itB)
    {
        if (*itA != *itB)
            return false;
    }

    return true;
}

void
Structure::merge(const Domain &value)
{
    const Structure &structure = dynCast<const Structure&>(value);
    CANAL_ASSERT(mMembers.size() == structure.mMembers.size());
    std::vector<Domain*>::iterator itA = mMembers.begin();
    std::vector<Domain*>::const_iterator itAend = mMembers.end(),
        itB = structure.mMembers.begin();
    for (; itA != itAend; ++itA, ++itB)
        (*itA)->merge(**itB);
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
    std::stringstream ss;
    ss << "structure" << std::endl;
    std::vector<Domain*>::const_iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        ss << indent((*it)->toString(), 4);

    return ss.str();
}

bool
Structure::matchesString(const std::string &text,
                         std::string &rationale) const
{
    CANAL_NOT_IMPLEMENTED();
}

std::vector<Domain*>
Structure::getItem(const Domain &offset) const
{
    std::vector<Domain*> result;
    const Integer::Container &integer =
        dynCast<const Integer::Container&>(offset);

    // First try an enumeration, then interval.
    const Integer::Enumeration &enumeration = integer.getEnumeration();
    if (!enumeration.isTop())
    {
        APIntUtils::USet::const_iterator it = enumeration.mValues.begin(),
            itend = enumeration.mValues.end();
        for (; it != itend; ++it)
        {
            CANAL_ASSERT(it->getBitWidth() <= 64);
            uint64_t numOffset = it->getZExtValue();

            // If some offset from the enumeration points out of the
            // array bounds, we ignore it FOR NOW.  It might be caused
            // either by a bug in the code, or by imprecision of the
            // interpreter.
            if (numOffset >= mMembers.size())
                continue;

            result.push_back(mMembers[numOffset]);
        }

        // At least one of the offsets in the enumeration should point
        // to the array.  Otherwise it might be a bug in the
        // interpreter that requires investigation.
        CANAL_ASSERT(!result.empty());
        return result;
    }

    const Integer::Interval &interval = integer.getInterval();
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

    // Both enumeration and interval are set to the top value, so return
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

    mMembers[offset]->merge(value);
}

} // namespace Canal
