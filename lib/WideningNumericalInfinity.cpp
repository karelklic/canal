#include "WideningNumericalInfinity.h"
#include "WideningDataIterationCount.h"
#include "IntegerContainer.h"
#include "FloatInterval.h"
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
    Float::Interval *f =
            dynCast<Float::Interval*>(&first);

    if (!firstContainer && !f)
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
    if (firstContainer) firstContainer->setTop();
    else f->setTop();
}

} // namespace Widening
} // namespace Canal
