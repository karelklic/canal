#include "Structure.h"
#include "Utils.h"
#include "Constant.h"
#include "IntegerContainer.h"
#include "IntegerEnumeration.h"
#include "IntegerRange.h"
#include <sstream>

namespace Canal {

Structure::Structure(const Structure &structure)
{
    mMembers = structure.mMembers;
    std::vector<Value*>::iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        *it = (*it)->clone();
}

Structure::~Structure()
{
    std::vector<Value*>::iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        delete (*it);
}

Structure *
Structure::clone() const
{
    return new Structure(*this);
}

bool
Structure::operator==(const Value &value) const
{
    const Structure *structure = dynamic_cast<const Structure*>(&value);
    if (!structure)
        return false;

    if (mMembers.size() != structure->mMembers.size())
        return false;

    std::vector<Value*>::const_iterator itA = mMembers.begin(),
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
Structure::merge(const Value &value)
{
    const Structure &structure = dynamic_cast<const Structure&>(value);
    CANAL_ASSERT(mMembers.size() == structure.mMembers.size());
    std::vector<Value*>::iterator itA = mMembers.begin();
    std::vector<Value*>::const_iterator itAend = mMembers.end(),
        itB = structure.mMembers.begin();
    for (; itA != itAend; ++itA, ++itB)
        (*itA)->merge(**itB);
}

size_t
Structure::memoryUsage() const
{
    size_t size = sizeof(Structure);
    size += mMembers.capacity() * sizeof(Value*);
    std::vector<Value*>::const_iterator it = mMembers.begin();
    for (; it != mMembers.end(); ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
Structure::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Structure: {" << std::endl;
    ss << "}";
    return ss.str();
}

std::vector<Value*>
Structure::getItem(const Value &offset) const
{
    std::vector<Value*> result;
    if (const Constant *constant = dynamic_cast<const Constant*>(&offset))
    {
        CANAL_ASSERT(constant->isAPInt());
        CANAL_ASSERT(constant->getAPInt().getBitWidth() <= 64);
        uint64_t numOffset = constant->getAPInt().getZExtValue();
        CANAL_ASSERT(numOffset < mMembers.size());
        result.push_back(mMembers[numOffset]);
        return result;
    }

    const Integer::Container &integer = dynamic_cast<const Integer::Container&>(offset);

    // First try an enumeration, then range.
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

    const Integer::Range &range = integer.getRange();
    // Let's care about the unsigned range only.
    if (!range.mUnsignedTop)
    {
        CANAL_ASSERT(range.mUnsignedFrom.getBitWidth() <= 64);
        uint64_t from = range.mUnsignedFrom.getZExtValue();
        // Included in the interval!
        uint64_t to = range.mUnsignedTo.getZExtValue();
        // At least part of the range should point to the array.
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

    // Both enumeration and range are set to the top value, so return
    // all members.
    result.insert(result.end(), mMembers.begin(), mMembers.end());

    // Zero length arrays are not supported.
    CANAL_ASSERT(!result.empty());
    return result;
}

Value *
Structure::getItem(uint64_t offset) const
{
    CANAL_ASSERT_MSG(offset < mMembers.size(),
                     "Offset out of bounds.");

    return mMembers[offset];
}

void
Structure::setItem(const Value &offset, const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Structure::setItem(uint64_t offset, const Value &value)
{
    CANAL_ASSERT_MSG(offset < mMembers.size(),
                     "Offset out of bounds.");

    mMembers[offset]->merge(value);
}

} // namespace Canal
