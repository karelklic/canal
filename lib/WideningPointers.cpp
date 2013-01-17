#include "WideningPointers.h"
#include "WideningDataIterationCount.h"
#include "Pointer.h"
#include "Utils.h"

namespace Canal {
namespace Widening {

void
Pointers::widen(const llvm::BasicBlock &wideningPoint,
                Domain &first,
                const Domain &second)
{
    Pointer::Pointer *firstPointer =
        llvm::dyn_cast<Pointer::Pointer>(&first);

    if (!firstPointer)
        return;

    //const Product::Vector &secondContainer =
    //    llvm::cast<Product::Vector>(second);

    DataInterface *data = first.getWideningData();
    DataIterationCount *iterationCount;
    if (data)
        iterationCount = llvm::cast<DataIterationCount>(data);
    else
    {
        iterationCount = new DataIterationCount();
        first.setWideningData(iterationCount);
    }

    iterationCount->increase(wideningPoint);

    if (iterationCount->count(wideningPoint) < DataIterationCount::ITERATION_COUNT)
        return;

    Pointer::PlaceTargetMap::const_iterator
        it = firstPointer->mTargets.begin(),
        itend = firstPointer->mTargets.end();

    for (; it != itend; ++it)
    {
        if (it->second->mNumericOffset)
            it->second->mNumericOffset->setTop();

        if (!it->second->mOffsets.empty())
        {
            // Keep the first offset untouched, it must be set to zero.
            std::vector<Domain*>::const_iterator
                itOffset = it->second->mOffsets.begin() + 1,
                itendOffset = it->second->mOffsets.end();

            for (; itOffset != itendOffset; ++itOffset)
                (*itOffset)->setTop();
        }
    }

}

} // namespace Widening
} // namespace Canal
