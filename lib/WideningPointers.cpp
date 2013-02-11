#include "WideningPointers.h"
#include "WideningDataIterationCount.h"
#include "MemoryPointer.h"
#include "Utils.h"

namespace Canal {
namespace Widening {

void
Pointers::widen(const llvm::BasicBlock &wideningPoint,
                Domain &first,
                const Domain &second)
{
    Memory::Pointer *firstPointer = dynCast<Memory::Pointer>(&first);
    if (!firstPointer)
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

    const Memory::Pointer &secondPointer =
        checkedCast<Memory::Pointer>(second);

    firstPointer->widen(secondPointer);
}

} // namespace Widening
} // namespace Canal
