#include "Structure.h"
#include <sstream>

namespace Canal {

Structure::Structure(const Structure &structure)
{
    mMembers = structure.mMembers;

    for (std::vector<Value*>::iterator it = mMembers.begin(); it != mMembers.end(); ++it)
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

    std::vector<Value*>::const_iterator itA = mMembers.begin(), itAend = mMembers.end(),
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
    for (std::vector<Value*>::const_iterator it = mMembers.begin(); it != mMembers.end(); ++it)
        size += (*it)->memoryUsage();

    return size;
}

std::string
Structure::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Structure: {" << std::endl;
    //ss << "    size:" << indentExceptFirstLine(mSize->toString(state), 9) << std::endl;
    //ss << "    value: " << indentExceptFirstLine(mValue->toString(state), 11) << std::endl;
    ss << "}";
    return ss.str();
}

} // namespace Canal
