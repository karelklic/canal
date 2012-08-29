#include "Pointer.h"
#include "ArraySingleItem.h"
#include "Constant.h"
#include "Utils.h"
#include "State.h"
#include "SlotTracker.h"
#include "IntegerContainer.h"
#include "Environment.h"
#include <llvm/BasicBlock.h>
#include <llvm/Type.h>
#include <sstream>

namespace Canal {
namespace Pointer {

InclusionBased::InclusionBased(const Environment &environment,
                               const llvm::Type &type)
    : Domain(environment), mType(type), mTop(false)
{
}

InclusionBased::InclusionBased(const InclusionBased &second)
    : Domain(second.mEnvironment),
      mType(second.mType),
      mTargets(second.mTargets),
      mTop(second.mTop)
{
    PlaceTargetMap::iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        it->second = new Target(*it->second);
}

InclusionBased::InclusionBased(const InclusionBased &second,
                               const llvm::Type &newType)
    : Domain(second.mEnvironment),
      mType(newType),
      mTargets(second.mTargets),
      mTop(second.mTop)
{
    PlaceTargetMap::iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        it->second = new Target(*it->second);
}

InclusionBased::~InclusionBased()
{
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        delete it->second;
}

void
InclusionBased::addTarget(Target::Type type,
                          const llvm::Value *instruction,
                          const llvm::Value *target,
                          const std::vector<Domain*> &offsets,
                          Domain *numericOffset)
{
    CANAL_ASSERT_MSG(instruction,
                     "Instruction is mandatory.");

    if (mTop)
        return;

    Target *pointerTarget = new Target(mEnvironment, type, target, offsets, numericOffset);

    PlaceTargetMap::iterator it = mTargets.find(instruction);
    if (it != mTargets.end())
    {
        it->second->merge(*pointerTarget);
        delete pointerTarget;
    }
    else
        mTargets.insert(PlaceTargetMap::value_type(instruction, pointerTarget));
}

Domain *
InclusionBased::dereferenceAndMerge(const State &state) const
{
    Domain *mergedValue = NULL;
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        std::vector<Domain*> values = it->second->dereference(state);
        std::vector<Domain*>::const_iterator it = values.begin();
        for (; it != values.end(); ++it)
        {
            if (mergedValue)
                mergedValue->merge(**it);
            else
                mergedValue = (*it)->clone();
        }
    }

    return mergedValue;
}

InclusionBased *
InclusionBased::bitcast(const llvm::Type &type) const
{
    return new InclusionBased(*this, type);
}

InclusionBased *
InclusionBased::getElementPtr(const std::vector<Domain*> &offsets,
                              const llvm::Type &type) const
{
    InclusionBased *result = new InclusionBased(*this, type);

    PlaceTargetMap::iterator targetIt = result->mTargets.begin();
    for (; targetIt != result->mTargets.end(); ++targetIt)
    {
        std::vector<Domain*>::const_iterator offsetIt = offsets.begin();
        for (; offsetIt != offsets.end(); ++offsetIt)
        {
            if (targetIt == result->mTargets.begin())
                targetIt->second->mOffsets.push_back(*offsetIt);
            else
                targetIt->second->mOffsets.push_back((*offsetIt)->clone());
        }
    }

    if (result->mTargets.empty())
    {
        std::vector<Domain*>::const_iterator offsetIt = offsets.begin();
        for (; offsetIt != offsets.end(); ++offsetIt)
            delete *offsetIt;
    }

    return result;
}

void
InclusionBased::store(const Domain &value, State &state)
{
    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        std::vector<Domain*> destinations = it->second->dereference(state);
        std::vector<Domain*>::iterator it = destinations.begin();
        for (; it != destinations.end(); ++it)
            (*it)->merge(value);
    }
}

InclusionBased *
InclusionBased::clone() const
{
    return new InclusionBased(*this);
}

InclusionBased *
InclusionBased::cloneCleaned() const
{
    return new InclusionBased(mEnvironment, mType);
}

bool
InclusionBased::operator==(const Domain &value) const
{
    // Check if the value has the same type.
    const InclusionBased *pointer =
        dynCast<const InclusionBased*>(&value);

    if (!pointer)
        return false;

    if (&pointer->mType != &mType)
        return false;

    if (pointer->mTop != mTop)
        return false;

    CANAL_ASSERT(!mTop || (pointer->mTargets.empty() && mTargets.empty()));

    // Check if it has the same number of targets.
    if (pointer->mTargets.size() != mTargets.size())
        return false;

    // Check the targets.
    PlaceTargetMap::const_iterator it1 = pointer->mTargets.begin(),
        it2 = mTargets.begin();
    for (; it2 != mTargets.end(); ++it1, ++it2)
    {
        if (it1->first != it2->first || *it1->second != *it2->second)
            return false;
    }

    return true;
}

bool
InclusionBased::isSingleTarget() const
{
    if (mTop || mTargets.size() != 1)
        return false;

    const Target *target = mTargets.begin()->second;

    const Integer::Container *tmp =
        dynCast<const Integer::Container*>(target->mNumericOffset);

    if (!tmp->isSingleValue())
        return false;

    std::vector<Domain*>::const_iterator it = target->mOffsets.begin();
    for (; it != target->mOffsets.end(); ++it)
    {
        tmp = dynCast<const Integer::Container*>(*it);
        if (!tmp->isSingleValue())
            return false;
    }

    return true;
}

void
InclusionBased::merge(const Domain &value)
{
    CANAL_ASSERT_MSG(!dynCast<const Constant*>(&value),
                     "Constant values are not supported for pointers."
                     "Getelementptr should be evaluated in the interpreter.");

    const InclusionBased &vv = dynCast<const InclusionBased&>(value);

    CANAL_ASSERT_MSG(&vv.mType == &mType,
                     "Unexpected different types in a pointer merge ("
                     << Canal::toString(vv.mType) << " != "
                     << Canal::toString(mType) << ")");

    if (mTop)
        return;

    if (vv.isTop())
        setTop();
    else
    {
        PlaceTargetMap::const_iterator valueit = vv.mTargets.begin();
        for (; valueit != vv.mTargets.end(); ++valueit)
        {
            PlaceTargetMap::iterator it = mTargets.find(valueit->first);
            if (it == mTargets.end())
                mTargets.insert(PlaceTargetMap::value_type(
                                    valueit->first, new Target(*valueit->second)));
            else
                it->second->merge(*valueit->second);
        }
    }
}

size_t
InclusionBased::memoryUsage() const
{
    size_t size = sizeof(InclusionBased);

    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        size += it->second->memoryUsage();

    return size;
}

std::string
InclusionBased::toString() const
{
    std::stringstream ss;
    ss << "pointer" << std::endl;
    ss << "    type " << Canal::toString(mType) << std::endl;
    if (mTop)
    {
        CANAL_ASSERT(mTargets.empty());
        ss << "    top" << std::endl;
    }
    else
    {
        PlaceTargetMap::const_iterator it = mTargets.begin();
        for (; it != mTargets.end(); ++it)
            ss << indent(it->second->toString(mEnvironment.mSlotTracker), 4);
    }

    return ss.str();
}

bool
InclusionBased::matchesString(const std::string &text,
                              std::string &rationale) const
{
    CANAL_NOT_IMPLEMENTED();
}

float
InclusionBased::accuracy() const
{
    return mTop ? 0.0f : 1.0f;
}

bool
InclusionBased::isBottom() const
{
    return !mTop && mTargets.empty();
}

void
InclusionBased::setBottom()
{
    mTop = false;
    mTargets.clear();
}

bool
InclusionBased::isTop() const
{
    return mTop;
}

void
InclusionBased::setTop()
{
    mTop = true;
    mTargets.clear();
}

} // namespace Pointer
} // namespace Canal
