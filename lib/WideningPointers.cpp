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
        dynCast<Pointer::Pointer*>(&first);

    if (!firstPointer)
        return;

    //const Integer::Container &secondContainer =
    //    dynCast<const Integer::Container&>(second);

    DataInterface *data = first.getWideningData();
    DataIterationCount *iterationCount;
    if (data)
        iterationCount = dynCast<DataIterationCount*>(data);
    else
    {
        iterationCount = new DataIterationCount();
        first.setWideningData(iterationCount);
    }

    CANAL_ASSERT_MSG(iterationCount,
                     "Invalid or conflicting widening data.");

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

        if (!it->second->mElementOffsets.empty())
        {
            // Keep the first offset untouched, it must be set to zero.
            std::vector<Domain*>::const_iterator
                itOffset = it->second->mElementOffsets.begin() + 1,
                itendOffset = it->second->mElementOffsets.end();

            for (; itOffset != itendOffset; ++itOffset)
                (*itOffset)->setTop();
        }
    }

}

} // namespace Widening
} // namespace Canal
