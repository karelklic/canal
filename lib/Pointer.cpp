#include "Pointer.h"
#include "ArraySingleItem.h"
#include "Constant.h"
#include "Utils.h"
#include "State.h"
#include "SlotTracker.h"
#include <llvm/BasicBlock.h>
#include <sstream>

namespace Canal {
namespace Pointer {

InclusionBased::InclusionBased(const llvm::Module &module,
                               const llvm::Type *type)
    : mModule(module), mType(type)
{
}

InclusionBased::InclusionBased(const InclusionBased &second)
    : mModule(second.mModule),
      mType(second.mType),
      mTargets(second.mTargets)
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
                          const std::vector<Value*> &offsets,
                          Value *numericOffset)
{
    CANAL_ASSERT_MSG(instruction,
                     "Instruction is mandatory.");

    Target *pointerTarget = new Target(type, target, offsets, numericOffset);

    PlaceTargetMap::iterator it = mTargets.find(instruction);
    if (it != mTargets.end())
    {
        it->second->merge(*pointerTarget);
        delete pointerTarget;
    }
    else
        mTargets.insert(PlaceTargetMap::value_type(instruction, pointerTarget));
}

Value *
InclusionBased::dereferenceAndMerge(const State &state) const
{
    Value *mergedValue = NULL;
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        std::vector<Value*> values = it->second->dereference(state);
        std::vector<Value*>::const_iterator it = values.begin();
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
InclusionBased::bitcast(const llvm::Type *type) const
{
    InclusionBased *result = clone();
    result->mType = type;
    return result;
}

InclusionBased *
InclusionBased::getElementPtr(const std::vector<Value*> &offsets,
                              const llvm::Type *type) const
{
    InclusionBased *result = clone();
    result->mType = type;

    PlaceTargetMap::iterator targetIt = result->mTargets.begin();
    for (; targetIt != result->mTargets.end(); ++targetIt)
    {
        std::vector<Value*>::const_iterator offsetIt = offsets.begin();
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
        std::vector<Value*>::const_iterator offsetIt = offsets.begin();
        for (; offsetIt != offsets.end(); ++offsetIt)
            delete *offsetIt;
    }

    return result;
}

void
InclusionBased::store(const Value &value, State &state)
{
    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        std::vector<Value*> destinations = it->second->dereference(state);
        std::vector<Value*>::iterator it = destinations.begin();
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
    return new InclusionBased(mModule, mType);
}

bool
InclusionBased::operator==(const Value &value) const
{
    // Check if the value has the same type.
    const InclusionBased *pointer =
        dynCast<const InclusionBased*>(&value);

    if (!pointer)
        return false;

    if (pointer->mType != mType)
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

void
InclusionBased::merge(const Value &value)
{
    CANAL_ASSERT_MSG(!dynCast<const Constant*>(&value),
                     "Constant values are not supported for pointers."
                     "Getelementptr should be evaluated in the interpreter.");

    const InclusionBased &vv = dynCast<const InclusionBased&>(value);

    CANAL_ASSERT_MSG(vv.mType == mType,
                     "Unexpected different types in a pointer merge ("
                     << *vv.mType << " != " << *mType << ")");

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
    SlotTracker slotTracker(mModule);
    std::stringstream ss;
    ss << "pointer" << std::endl;
    ss << "    type " << Canal::toString(*mType) << std::endl;
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        ss << indent(it->second->toString(slotTracker), 4);

    return ss.str();
}

} // namespace Pointer
} // namespace Canal
