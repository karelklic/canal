#include "WideningNumericalInfinity.h"
#include "WideningDataIterationCount.h"
#include "IntegerContainer.h"
#include "Utils.h"

namespace Canal {
namespace Widening {

void
NumericalInfinity::widen(const llvm::BasicBlock &wideningPoint,
                         Domain &first,
                         const Domain &second)
{
    Integer::Container *firstContainer =
        dynCast<Integer::Container*>(&first);

    if (!firstContainer)
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

    // Widening.
    firstContainer->setTop();
}

} // namespace Widening
} // namespace Canal
