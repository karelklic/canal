#include "WideningNumericalInfinity.h"
#include "WideningDataIterationCount.h"

namespace Canal {
namespace Widening {

void
NumericalInfinity::widen(const llvm::BasicBlock &wideningPoint,
                         Domain &first,
                         const Domain &second)
{
    Data::Interface *data = first.getWideningData();
    Data::IterationCount *iterationCount;
    if (data)
        iterationCount = dynCast<Data::IterationCount*>(data);
    else
        iterationCount = new Data::IterationCount();

    CANAL_ASSERT_MSG(iterationCount,
                     "Invalid or conflicting widening data.");

    
}

} // namespace Widening
} // namespace Canal
