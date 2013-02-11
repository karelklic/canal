#include "WideningManager.h"
#include "WideningNumericalInfinity.h"
#include "WideningPointers.h"
#include "MemoryState.h"
#include "MemoryBlockMap.h"
#include "MemoryVariableMap.h"
#include "Domain.h"
#if 0 // Debug info for fixpoint calculation
#include "Utils.h"
#include "Environment.h"
#include <iostream>
#endif

namespace Canal {
namespace Widening {

Manager::Manager()
{
    mWidenings.push_back(new NumericalInfinity());
    mWidenings.push_back(new Pointers());
}

Manager::~Manager()
{
    llvm::DeleteContainerPointers(mWidenings);
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               Memory::State &first,
               const Memory::State &second) const
{
    widen(wideningPoint,
          first.getVariables(),
          second.getVariables());

    widen(wideningPoint,
          first.getBlocks(),
          second.getBlocks());
}

template <typename T>
void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               Memory::Map<T> &first,
               const Memory::Map<T> &second) const
{
    typename Memory::Map<T>::const_iterator it2 = second.begin(),
        it2end = second.end();

    for (; it2 != it2end; ++it2)
    {
        typename Memory::Map<T>::iterator it1 = first.find(it2->first);
	if (it1 != first.end() && *it1->second != *it2->second)
        {
#if 0 // Debug info for fixpoint calculation
            std::cout << ((it1->second)->toString()) << "\n";
            std::cout << Canal::getName(*it1->first,
                                        it1->second->getEnvironment().getSlotTracker())
                      << "\n";

            std::cout << ((it2->second)->toString()) << "\n";
#endif

            widen(wideningPoint, *it1->second.mutable_(), *it2->second);
        }
    }
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               Domain &first,
               const Domain &second) const
{
    std::vector<Interface*>::const_iterator
        it = mWidenings.begin(), itend = mWidenings.end();

    for (; it != itend; ++it)
        (*it)->widen(wideningPoint, first, second);
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               Memory::Block &first,
               const Memory::Block &second) const
{
    std::vector<Interface*>::const_iterator
        it = mWidenings.begin(), itend = mWidenings.end();

    for (; it != itend; ++it)
    {
        (*it)->widen(wideningPoint,
                     first.getMainValue(),
                     second.getMainValue());
    }
}

} // namespace Widening
} // namespace Canal
