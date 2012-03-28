#include "AbstractPointer.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>

AbstractPointer::~AbstractPointer()
{
    std::set<AbstractValue*>::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        delete *it;
}

AbstractPointer* AbstractPointer::clone() const
{
    AbstractPointer *dup = new AbstractPointer();
    std::set<AbstractValue*>::iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        dup->mTargets.insert((*it)->clone());
}

bool AbstractPointer::operator==(const AbstractValue &value) const
{
    // Check if rhs has the same type.
    const AbstractPointer *pointer = dynamic_cast<const AbstractPointer*>(&value);
    if (!pointer)
        return false;

    return true;
}

void AbstractPointer::merge(const AbstractValue &value)
{
    const AbstractPointer &vv = dynamic_cast<const AbstractPointer&>(value);
    std::set<AbstractValue*>::iterator it = vv.mTargets.begin();
    for (; it != vv.mTargets.end(); ++it)
    {
        if (mTargets.find(*it) != mTargets.end())
            continue;

        mTargets.insert((*it)->clone());
    }
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

void AbstractPointer::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "AbstractPointer(size: " << mTargets.size() << "items)";
}
