#include "Pointer.h"
#include "ArraySingleItem.h"
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
    : Domain(environment), mType(type)
{
}

InclusionBased::InclusionBased(const InclusionBased &value)
    : Domain(value),
      mTargets(value.mTargets),
      mType(value.mType)
{
    PlaceTargetMap::iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        it->second = new Target(*it->second);
}

InclusionBased::InclusionBased(const InclusionBased &value,
                               const llvm::Type &newType)
    : Domain(value),
      mTargets(value.mTargets),
      mType(newType)
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
                          const llvm::Value *place,
                          const llvm::Value *target,
                          const std::vector<Domain*> &offsets,
                          Domain *numericOffset)
{
    CANAL_ASSERT_MSG(place,
                     "Place is mandatory.");

    CANAL_ASSERT_MSG(llvm::isa<llvm::Instruction>(place) ||
                     llvm::isa<llvm::GlobalValue>(place),
                     "Place must be either an instruction or a global value.");

    Target *pointerTarget = new Target(mEnvironment,
                                       type,
                                       target,
                                       offsets,
                                       numericOffset);

    PlaceTargetMap::iterator it = mTargets.find(place);
    if (it != mTargets.end())
    {
        it->second->merge(*pointerTarget);
        delete pointerTarget;
    }
    else
        mTargets.insert(PlaceTargetMap::value_type(place, pointerTarget));
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
    CANAL_ASSERT_MSG(!offsets.empty(),
                     "getElementPtr must be called with some offsets.");

    // Check that all offsets are 64-bit integers.
    std::vector<Domain*>::const_iterator offsetIt = offsets.begin();
    for (; offsetIt != offsets.end(); ++offsetIt)
    {
        const Integer::Container &container =
            dynCast<const Integer::Container&>(**offsetIt);

        CANAL_ASSERT_MSG(container.getBitWidth() == 64,
                         "GetElementPtr offsets must have 64 bits!");
    }

    InclusionBased *result = new InclusionBased(*this, type);

    // Iterate over all targets, and adjust the target offsets.
    PlaceTargetMap::iterator targetIt = result->mTargets.begin();
    for (; targetIt != result->mTargets.end(); ++targetIt)
    {
        std::vector<Domain*> &targetOffsets = targetIt->second->mOffsets;
        std::vector<Domain*>::const_iterator offsetIt = offsets.begin();
        for (; offsetIt != offsets.end(); ++offsetIt)
        {
            if (offsetIt == offsets.begin() && !targetOffsets.empty())
            {
                Domain *oldLast = targetOffsets.back();
                Domain *newLast = oldLast->cloneCleaned();
                newLast->add(*oldLast, **offsets.begin());
                delete oldLast;
                targetOffsets.pop_back();
                targetOffsets.push_back(newLast);
                continue;
            }

            targetOffsets.push_back((*offsetIt)->clone());
        }
    }

    // Delete the offsets, because this method takes ownership of them
    // and it no longer needs them.
    offsetIt = offsets.begin();
    for (; offsetIt != offsets.end(); ++offsetIt)
        delete *offsetIt;

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
    if (mTargets.size() != 1)
        return false;

    const Target *target = mTargets.begin()->second;

    const Integer::Container *tmp =
        dynCast<const Integer::Container*>(target->mNumericOffset);

    if (tmp && !tmp->isSingleValue())
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
    const InclusionBased &vv = dynCast<const InclusionBased&>(value);
    CANAL_ASSERT_MSG(&vv.mType == &mType,
                     "Unexpected different types in a pointer merge ("
                     << Canal::toString(vv.mType) << " != "
                     << Canal::toString(mType) << ")");

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
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        ss << indent(it->second->toString(mEnvironment.getSlotTracker()), 4);

    return ss.str();
}

void
InclusionBased::setZero(const llvm::Value *instruction)
{
    mTargets.clear();
    addTarget(Pointer::Target::Constant,
              instruction,
              NULL,
              std::vector<Domain*>(),
              NULL);
}

} // namespace Pointer
} // namespace Canal
