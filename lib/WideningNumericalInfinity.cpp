#include "WideningNumericalInfinity.h"
#include "WideningDataIterationCount.h"
#include "ProductVector.h"
#include "FloatInterval.h"
#include "Utils.h"

namespace Canal {
namespace Widening {

void
NumericalInfinity::widen(const llvm::BasicBlock &wideningPoint,
                         Domain &first,
                         const Domain &second)
{
    Product::Vector *firstContainer =
        dynCast<Product::Vector>(&first);

    Float::Interval *f = dynCast<Float::Interval>(&first);
    if (!firstContainer && !f)
        return;

    DataInterface *data = first.getWideningData();
    DataIterationCount *iterationCount;
    if (data)
        iterationCount = checkedCast<DataIterationCount>(data);
    else
    {
        iterationCount = new DataIterationCount();
        first.setWideningData(iterationCount);
    }

    iterationCount->increase(wideningPoint);

    if (iterationCount->count(wideningPoint) < DataIterationCount::ITERATION_COUNT)
        return;

    // Widening.
    if (firstContainer) firstContainer->setTop();
    else f->setTop();
}

} // namespace Widening
} // namespace Canal
