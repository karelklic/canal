#include "StateMap.h"
#include "Domain.h"
#include "Utils.h"

namespace Canal {

StateMap::StateMap(const StateMap &map) : mMap(map.mMap)
{
    for (iterator it = begin(); it != end(); ++it)
        it->second = it->second->clone();
}

StateMap::~StateMap()
{
    llvm::DeleteContainerSeconds(*this);
}

bool
StateMap::operator==(const StateMap &map) const
{
    if (&map == this)
        return true;

    if (size() != map.size())
        return false;

    for (const_iterator it = begin(); it != end(); ++it)
    {
        StateMap::const_iterator mapit = map.find(it->first);
	if (mapit == map.end() || *it->second != *mapit->second)
            return false;
    }

    return true;
}

void
StateMap::merge(const StateMap &map)
{
    const_iterator it2 = map.begin(), it2end = map.end();
    for (; it2 != it2end; ++it2)
    {
	iterator it1 = find(it2->first);
	if (it1 == end())
        {
            insert(value_type(it2->first,
                              it2->second->clone()));
        }
    else if (*it1->second != *it2->second)
            it1->second->merge(*it2->second);
    }
}

void
StateMap::insert(const llvm::Value &place, Domain *value)
{
    CANAL_ASSERT_MSG(value,
                     "Attempted to insert NULL variable to state.");

    iterator it = find(&place);
    if (it != end())
    {
        it->second->merge(*value);
        delete value;
    }
    else
        insert(value_type(&place, value));
}

size_t
StateMap::memoryUsage() const
{
    size_t result = sizeof(StateMap) + mMap.size() * (sizeof(const llvm::Value*) + sizeof(Domain*));
    for (const_iterator it = begin(); it != end(); ++it)
        result += it->second->memoryUsage();

    return result;
}

} // namespace Canal
