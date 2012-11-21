#include "WideningManager.h"
#include "WideningNumericalInfinity.h"
#include "WideningPointers.h"
#include "State.h"
#include "StateMap.h"
#include "Domain.h"
#if 0 //Debug info for fixpoint calculation
#include "Utils.h"
#include "Environment.h"
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
               State &first,
               const State &second) const
{
    widen(wideningPoint,
          first.getFunctionVariables(),
          second.getFunctionVariables());

    widen(wideningPoint,
          first.getFunctionBlocks(),
          second.getFunctionBlocks());

    widen(wideningPoint,
          first.getGlobalVariables(),
          second.getGlobalVariables());

    widen(wideningPoint,
          first.getGlobalBlocks(),
          second.getGlobalBlocks());
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               StateMap &first,
               const StateMap &second) const
{
    StateMap::const_iterator it2 = second.begin(),
        it2end = second.end();

    for (; it2 != it2end; ++it2)
    {
	StateMap::iterator it1 = first.find(it2->first);
	if (it1 != first.end() && *const_cast<const SharedDataPointer<Domain>&>(it1->second) != *it2->second)
        {
#if 0 //Debug info for fixpoint calculation
            std::cout << ((it1->second)->toString()) << "\n";
            std::cout << Canal::getName(*it1->first,
                                        it1->second->getEnvironment().getSlotTracker())
                      << "\n";

            std::cout << ((it2->second)->toString()) << "\n";
#endif

            widen(wideningPoint, *it1->second, *it2->second);
        }
    }
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               Domain &first,
               const Domain &second) const
{
    std::vector<Interface*>::const_iterator it = mWidenings.begin();
    for (; it != mWidenings.end(); ++it)
        (*it)->widen(wideningPoint, first, second);
}

} // namespace Widening
} // namespace Canal
