#include "WideningManager.h"
#include "WideningNumericalInifinity.h"

namespace Canal {
namespace Widening {

Manager::Manager()
{
    mWidenings.push_back(new NumericalInfinity());
}

Manager::~Manager()
{
    std::vector<Interface*>::const_iterator it = mWidenings.begin();
    for (; it != mWidenings.end(); ++it)
        delete *it;
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               State &first,
               const State &second) const
{
    widen(first.mFunctionVariables, second.mFunctionVariables);
    widen(first.mFunctionBlocks, second.mFunctionBlocks);
    widen(first.mGlobalVariables, second.mGlobalVariables);
    widen(first.mGlobalBlocks, second.mGlobalBlocks);
}

void
Manager::widen(const llvm::BasicBlock &wideningPoint,
               PlaceValueMap &first,
               const PlaceValueMap &second)
{
    PlaceValueMap::const_iterator it2 = second.begin(),
        it2end = second.end();

    for (; it2 != it2end; ++it2)
    {
	PlaceValueMap::iterator it1 = first.find(it2->first);
	if (it1 != first.end() && *it1->second != *it2->second)
            widen(*it1->second, *it2->second);
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
