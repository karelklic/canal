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

    if (iterationCount->count(wideningPoint) < 2)
        return;

    Pointer::PlaceTargetMap::const_iterator it = firstPointer->mTargets.begin();
    for (; it != firstPointer->mTargets.end(); ++it)
    {
        if (it->second->mNumericOffset)
        {
            AccuracyDomain *domain =
                dynCast<AccuracyDomain*>(it->second->mNumericOffset);

            if (domain)
                domain->setTop();
        }

        if (!it->second->mOffsets.empty())
        {
            // Keep the first offset untouched, it must be set to zero.
            std::vector<Domain*>::iterator itOffset =
                it->second->mOffsets.begin() + 1;

            for (; itOffset != it->second->mOffsets.end(); ++itOffset)
            {
                AccuracyDomain *domain = dynCast<AccuracyDomain*>(*itOffset);
                if (domain)
                    domain->setTop();
            }
        }
    }

}

} // namespace Widening
} // namespace Canal
