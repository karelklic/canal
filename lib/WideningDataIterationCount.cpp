#include "WideningDataIterationCount.h"

namespace Canal {
namespace Widening {
unsigned count = 2;
void
DataIterationCount::increase(const llvm::BasicBlock &block)
{
    mCounts[&block] = count(block) + 1;
}

int
DataIterationCount::count(const llvm::BasicBlock &block) const
{
    std::map<const llvm::BasicBlock*, int>::const_iterator it;
    it = mCounts.find(&block);
    return it == mCounts.end() ? 0 : it->second;
}

DataIterationCount *
DataIterationCount::clone() const
{
    return new DataIterationCount(*this);
}

} // namespace Widening
} // namespace Canal
