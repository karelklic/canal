#ifndef LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H
#define LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H

#include <map>
#include "WideningDataInterface.h"

namespace llvm {
class BasicBlock;
} // namespace llvm

namespace Canal {
namespace Widening {
extern unsigned count;
class DataIterationCount : public DataInterface
{
protected:
    std::map<const llvm::BasicBlock*, int> mCounts;

public:
    void increase(const llvm::BasicBlock &block);

    int count(const llvm::BasicBlock &block) const;

public:
    virtual DataIterationCount *clone() const;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H
