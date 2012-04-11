#include "Pointer.h"
#include "Utils.h"
#include "State.h"
#include <llvm/Support/raw_ostream.h>
#include <algorithm>

namespace Canal {
namespace Pointer {

Target::Target() : mType(Target::Uninitialized),
                   mArrayOffset(NULL),
                   mParent(NULL)
{
}

Target::Target(const Target &target) : mType(target.mType),
                                       mParent(target.mParent),
                                       mConstant(target.mConstant),
                                       mArrayOffset(target.mArrayOffset)
{
    if (mArrayOffset)
        mArrayOffset = mArrayOffset->clone();
}

Target::~Target()
{
    delete mArrayOffset;
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
    case GlobalBlock:
    case FunctionBlock:
        // Check array offset.
        if ((mArrayOffset && !target.mArrayOffset) ||
            (!mArrayOffset && target.mArrayOffset))
            return false;

        if (mArrayOffset && *mArrayOffset != *target.mArrayOffset)
            return false;

        return *dereference() == *target.dereference();
    default:
        CANAL_DIE();
    }

    return true;
}

bool Target::operator!=(const Target &target) const
{
    return !(*this == target);
}

Value *Target::dereference() const
{
    switch (mType)
    {
    case Uninitialized:
    case Constant:
        return NULL;
    case GlobalBlock:
        return mParent->getState()->mGlobalBlocks[mOffset];
    case FunctionBlock:
        return mParent->getState()->mFunctionBlocks[mOffset];
    default:
        CANAL_DIE();
    }
}

void Target::merge(const Target &target)
{
    CANAL_ASSERT(mType == target.mType);
    switch (mType)
    {
    case Uninitialized:
        break;
    case Constant:
        // TODO: mConstant can be abstract value.
        CANAL_ASSERT(mConstant == target.mConstant);
        break;
    case GlobalBlock:
    case FunctionBlock:
        dereference()->merge(*target.dereference());

        if (mArrayOffset)
        {
            if (target.mArrayOffset)
                mArrayOffset->merge(*target.mArrayOffset);
            else
                CANAL_NOT_IMPLEMENTED();
        }
        else if (target.mArrayOffset)
            CANAL_NOT_IMPLEMENTED();
        break;
    default:
        CANAL_DIE();
    }
}

InclusionBased* InclusionBased::clone() const
{
    return new InclusionBased(*this);
}

bool InclusionBased::operator==(const Value &value) const
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

void InclusionBased::merge(const Value &value)
{
    const InclusionBased &vv = dynamic_cast<const InclusionBased&>(value);
    std::map<const llvm::Value*, Target>::const_iterator valueit = vv.mTargets.begin();
    for (; valueit != vv.mTargets.end(); ++valueit)
    {
        std::map<const llvm::Value*, Target>::iterator it = mTargets.find(valueit->first);
        if (it == mTargets.end())
            mTargets.insert(*valueit);
        else
            it->second.merge(valueit->second);
    }
}

size_t InclusionBased::memoryUsage() const
{
    return mTargets.size() * sizeof(Target);
}

void InclusionBased::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Pointer::InclusionBased(size: " << mTargets.size() << "items)";
}

} // namespace Pointer
} // namespace Canal
