#include "AbstractPointer.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>

AbstractPointer* AbstractPointer::clone() const
{
    return new AbstractPointer(*this);
}

void AbstractPointer::merge(const AbstractValue &v)
{
    const AbstractPointer &vv = dynamic_cast<const AbstractPointer&>(v);
    std::set<AbstractValue*> temp;
    std::set_union(mTargets.begin(), mTargets.end(),
                   vv.mTargets.begin(), vv.mTargets.end(),
                   std::inserter(temp, temp.begin()));
    mTargets.swap(temp);
}

size_t AbstractPointer::memoryUsage() const
{
    return mTargets.size() * sizeof(AbstractValue*);
}

bool AbstractPointer::limitMemoryUsage(size_t size)
{
    // Memory usage of this value cannot be lowered.
    return false;
}

float AbstractPointer::accuracy() const
{
    // Not possible to determine.
    return 1.0;
}

void AbstractPointer::printToStream(llvm::raw_ostream &o) const
{
    o << "AbstractPointer(size: " << mTargets.size() << "items)";
}
