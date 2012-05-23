#include "Structure.h"
#include "Utils.h"
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
    for (std::vector<Value*>::iterator it = mMembers.begin(); it != mMembers.end(); ++it)
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

} // namespace Canal
