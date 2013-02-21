#include "MemoryBlockMap.h"
#include "Utils.h"

namespace Canal {
namespace Memory {

static bool
containsPlace(const llvm::BasicBlock &basicBlock,
              const llvm::Value *place)
{
    llvm::BasicBlock::const_iterator it = basicBlock.begin(),
        itend = basicBlock.end();

    for (; it != itend; ++it)
    {
        if (it == place)
            return true;
    }

    return false;
}

static bool
containsPlace(const llvm::Function &function,
              const llvm::Value *place)
{
    llvm::Function::const_iterator it = function.begin(),
        itend = function.end();

    for (; it != itend; ++it)
    {
        if (containsPlace(*it, place))
            return true;
    }

    return false;
}

void
BlockMap::join(const BlockMap &map,
               Block::MemoryType memoryType)
{
    const_iterator it2 = map.begin(), it2end = map.end();
    for (; it2 != it2end; ++it2)
    {
        if (it2->second->getMemoryType() != memoryType)
            continue;

	iterator it1 = find(it2->first);
	if (it1 == end())
            insert(*it2);
	else if (*it1->second != *it2->second)
            it1->second.mutable_()->join(*it2->second);
    }
}

void
BlockMap::joinForeignStack(const BlockMap &map,
                           const llvm::Function &currentFunction)
{
    const_iterator it2 = map.begin(), it2end = map.end();
    for (; it2 != it2end; ++it2)
    {
        if (containsPlace(currentFunction, it2->first))
            continue;

	iterator it1 = find(it2->first);
	if (it1 == end())
            insert(*it2);
	else if (*it1->second != *it2->second)
            it1->second.mutable_()->join(*it2->second);
    }
}

void
BlockMap::findNames(const std::string &prefix,
                    Block::MemoryType memoryType,
                    SlotTracker &slotTracker,
                    std::vector<std::string> &result) const
{
    const_iterator it = begin(), itend = end();
    for (; it != itend; ++it)
    {
        if (it->second->getMemoryType() != memoryType)
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
