#include "MemoryBlockMap.h"

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
BlockMap::joinForeignStack(const BlockMap &map,
                           const llvm::Function &currentFunction)
{
    // Merge function blocks that do not belong to current function.
    const_iterator it2 = map.begin(), it2end = map.end();

    for (; it2 != it2end; ++it2)
    {
	iterator it1 = find(it2->first);
	if (it1 == end())
        {
            if (containsPlace(currentFunction, it2->first))
                continue;

            insert(*it2);
        }
	else if (*it1->second != *it2->second)
            it1->second.mutable_()->join(*it2->second);
    }

}


} // namespace Memory
} // namespace Canal
