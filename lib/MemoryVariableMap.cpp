#include "MemoryVariableMap.h"
#include "MemoryUtils.h"
#include "Utils.h"

namespace Canal {
namespace Memory {

void
VariableMap::joinGlobals(const VariableMap &map)
{
    const_iterator it2 = map.begin(), it2end = map.end();
    for (; it2 != it2end; ++it2)
    {
        if (!Utils::isGlobal(*it2->first))
            continue;

        iterator it1 = find(it2->first);
        if (it1 == end())
            insert(*it2);
        else if (*it1->second != *it2->second)
            it1->second.mutable_()->join(*it2->second);
    }
}

void
VariableMap::findNames(const std::string &prefix,
                       bool isGlobal,
                       SlotTracker &slotTracker,
                       std::vector<std::string> &result) const
{
    const_iterator it = begin(), itend = end();
    for (; it != itend; ++it)
    {
        if (Utils::isGlobal(*it->first) != isGlobal)
            continue;

        std::string name = Canal::getName(*it->first, slotTracker);

        if (0 == strncmp(name.c_str(), prefix.c_str(), prefix.length()))
        {
            result.push_back(name);
            continue;
        }

        // Try to add the function prefix.
        const llvm::Instruction *instruction =
            Canal::dynCast<llvm::Instruction>(it->first);

        if (instruction)
        {
            const llvm::Function &function =
                *instruction->getParent()->getParent();

            name = function.getName().str() + ":" + name;

            if (0 == strncmp(name.c_str(), prefix.c_str(), prefix.length()))
            {
                result.push_back(name);
                continue;
            }
        }
    }
}

} // namespace Memory
} // namespace Canal
