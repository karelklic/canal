#include "AbstractPointer.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>

namespace AbstractPointer {

Target::Target() : mType(Target::Uninitialized), mIsArrayOffset(false)
{
}

void Target::setBlockOffset(size_t offset, Target::Type type)
{
    assert(type == GlobalBlock || type == FunctionBlock);
    mOffset = offset;
    mType = type;
}

size_t Target::getBlockOffset() const
{
    assert(mType == GlobalBlock || mType == FunctionBlock);
    return mOffset;
}

void Target::setVariable(const llvm::Value *variable, Target::Type type)
{
    assert(variable && (type == GlobalVariable || type == FunctionVariable));
    mVariable = variable;
    mmType = type;
}

const llvm::Value *Target::getVariable() const
{
    assert(mType == GlobalVariable || mType == FunctionVariable);
    return mVariable;
}

void Target::setConstant(size_t constant)
{
    mConstant = constant;
    mType = Constant;
}

size_t Target::getConstant() const
{
    assert(mType == Constant);
    return mConstant;
}

void Target::setArrayOffset(size_t minOffset, size_t maxOffset)
{
    mIsArrayOffset = true;
    mMinArrayOffset = minOffset;
    mMaxArrayOffset = maxOffset;
}

InclusionBased* InclusionBased::clone() const
{
    return new InclusionBased(*this);
}

bool InclusionBased::operator==(const AbstractValue &value) const
{
    // Check if rhs has the same type.
    const InclusionBased *pointer = dynamic_cast<const InclusionBased*>(&value);
    if (!pointer)
        return false;

    // Check if it has the same number of targets.
    if (pointer->mTargets.size() != mTargets.size())
        return false;

    // Check the targets.
    std::set<Target>::const_iterator it1 = pointer->mTargets.begin(),
        it2 = mTargets.begin();
    for (; it2 != mTargets.end(); ++it1, ++it2)
    {
        if (*it1 != *it2)
            return false;
    }

    return true;
}

void InclusionBased::merge(const AbstractValue &value)
{
    const InclusionBased &vv = dynamic_cast<const InclusionBased&>(value);
    std::set<AbstractValue*>::iterator it = vv.mTargets.begin();
    for (; it != vv.mTargets.end(); ++it)
    {
        if (mTargets.find(*it) != mTargets.end())
            continue;

        mTargets.insert(*it);
    }
}

size_t InclusionBased::memoryUsage() const
{
    return mTargets.size() * sizeof(AbstractValue*);
}

bool InclusionBased::limitMemoryUsage(size_t size)
{
    // Memory usage of this value cannot be lowered.
    return false;
}

float InclusionBased::accuracy() const
{
    // Not possible to determine.
    return 1.0;
}

void InclusionBased::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "InclusionBased(size: " << mTargets.size() << "items)";
}

} // namespace AbstractPointer
