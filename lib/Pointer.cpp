#include "AbstractPointer.h"
#include "Utils.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>

namespace Canal {
namespace AbstractPointer {

Target::Target() : mType(Target::Uninitialized), mIsArrayOffset(false)
{
}

bool Target::operator==(const Target &target) const
{
    if (mType != target.mType)
        return false;

    switch (mType)
    {
    case Uninitialized:
        return true;
    case Constant:
        return mConstant == target.mConstant;
    case GlobalVariable:
    case FunctionVariable:
        // Check array offset.
        if (mIsArrayOffset != target.mIsArrayOffset ||
            (mIsArrayOffset &&
             (mMinArrayOffset != target.mMinArrayOffset ||
              mMaxArrayOffset != target.mMaxArrayOffset)))
            return false;

        return mVariable == target.mVariable;
    case GlobalBlock:
    case FunctionBlock:
        // Check array offset.
        if (mIsArrayOffset != target.mIsArrayOffset ||
            (mIsArrayOffset &&
             (mMinArrayOffset != target.mMinArrayOffset ||
              mMaxArrayOffset != target.mMaxArrayOffset)))
            return false;

        return mOffset == target.mOffset;
    default:
        CANAL_DIE();
    }

    return true;
}

bool Target::operator!=(const Target &target) const
{
    return !(*this == target);
}

void Target::setBlockOffset(size_t offset, Target::Type type)
{
    CANAL_ASSERT(type == GlobalBlock || type == FunctionBlock);
    mOffset = offset;
    mType = type;
}

size_t Target::getBlockOffset() const
{
    CANAL_ASSERT(mType == GlobalBlock || mType == FunctionBlock);
    return mOffset;
}

void Target::setVariable(const llvm::Value *variable, Target::Type type)
{
    CANAL_ASSERT(variable && (type == GlobalVariable ||
                              type == FunctionVariable));
    mVariable = variable;
    mType = type;
}

const llvm::Value *Target::getVariable() const
{
    CANAL_ASSERT(mType == GlobalVariable || mType == FunctionVariable);
    return mVariable;
}

void Target::setConstant(size_t constant)
{
    mConstant = constant;
    mType = Constant;
}

size_t Target::getConstant() const
{
    CANAL_ASSERT(mType == Constant);
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
    std::map<const llvm::Value*, Target>::const_iterator it1 = pointer->mTargets.begin(),
        it2 = mTargets.begin();
    for (; it2 != mTargets.end(); ++it1, ++it2)
    {
        if (it1->first != it2->first || it1->second != it2->second)
            return false;
    }

    return true;
}

void InclusionBased::merge(const AbstractValue &value)
{
    const InclusionBased &vv = dynamic_cast<const InclusionBased&>(value);
    std::map<const llvm::Value*, Target>::const_iterator valueit = vv.mTargets.begin();
    for (; it != vv.mTargets.end(); ++it)
    {
        std::map<const llvm::Value*, Target>::const_iterator it = mTargets.find(valueit->first);
        if (it == mTargets.end())
            mTargets.insert(*valueit);
        else
        {
            it->second->merge(valueit->second);
        }

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

bool InclusionBased::isBottom() const
{
    return mTargets.empty();
}

void InclusionBased::setTop()
{
    CANAL_FATAL_ERROR("neither implemented nor allowed!";
}

void InclusionBased::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "InclusionBased(size: " << mTargets.size() << "items)";
}

} // namespace AbstractPointer
} // namespace Canal
