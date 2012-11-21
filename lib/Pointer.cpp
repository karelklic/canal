#include "Pointer.h"
#include "ArraySingleItem.h"
#include "Utils.h"
#include "State.h"
#include "SlotTracker.h"
#include "IntegerContainer.h"
#include "IntegerEnumeration.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Pointer {

Pointer::Pointer(const Environment &environment,
                 const llvm::Type &type)
    : Domain(environment), mType(type)
{
}

Pointer::Pointer(const Pointer &value)
    : Domain(value),
      mTargets(value.mTargets),
      mType(value.mType)
{
    PlaceTargetMap::iterator it = mTargets.begin(), itend = mTargets.end();
    for (; it != itend; ++it)
        it->second = new Target(*it->second);
}

Pointer::Pointer(const Pointer &value,
                 const llvm::Type &newType)
    : Domain(value),
      mTargets(value.mTargets),
      mType(newType)
{
    PlaceTargetMap::iterator it = mTargets.begin(), itend = mTargets.end();
    for (; it != itend; ++it)
        it->second = new Target(*it->second);
}

Pointer::~Pointer()
{
    llvm::DeleteContainerSeconds(mTargets);
}

void
Pointer::addTarget(Target::Type type,
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

/// Dereference the target in a certain block.  Dereferencing might
/// result in multiple values being returned due to the nature of
/// offsets (offsets might include integer intervals).  The returned
/// pointers point to the memory owned by the block.
static std::vector<Domain*>
dereference(Domain *block,
            const std::vector<Domain*> &offsets)
{
    std::vector<Domain*> result;
    result.push_back(block);

    if (!offsets.empty())
    {
        const Integer::Container &first =
            dynCast<const Integer::Container&>(*offsets[0]);
        const Integer::Enumeration &firstEnumeration = first.getEnumeration();

        CANAL_ASSERT_MSG(firstEnumeration.mValues.size() == 1,
                         "First offset is expected to be zero!");

        CANAL_ASSERT_MSG(firstEnumeration.mValues.begin()->isMinValue(),
                         "First offset is expected to be zero!");

        std::vector<Domain*>::const_iterator itOffsets = offsets.begin() + 1;
        for (; itOffsets != offsets.end(); ++itOffsets)
        {
            std::vector<Domain*> nextLevelResult;
            std::vector<Domain*>::const_iterator itItems = result.begin();
            for (; itItems != result.end(); ++itItems)
            {
                std::vector<Domain*> items;
                Array::Interface &array =
                    dynCast<Array::Interface&>(**itItems);

                items = array.getItem(**itOffsets);
                nextLevelResult.insert(nextLevelResult.end(),
                                       items.begin(),
                                       items.end());
            }

            result.swap(nextLevelResult);
        }
    }

    return result;
}


/// Dereference the target in a certain block.  Dereferencing might
/// result in multiple values being returned due to the nature of
/// offsets (offsets might include integer intervals).  The returned
/// pointers point to the memory owned by the block.
static std::vector<const Domain*>
dereference(const Domain *block,
            const std::vector<Domain*> &offsets)
{
    std::vector<const Domain*> result;
    result.push_back(block);

    if (!offsets.empty())
    {
        const Integer::Container &first =
            dynCast<const Integer::Container&>(*offsets[0]);
        const Integer::Enumeration &firstEnumeration = first.getEnumeration();

        CANAL_ASSERT_MSG(firstEnumeration.mValues.size() == 1,
                         "First offset is expected to be zero!");

        CANAL_ASSERT_MSG(firstEnumeration.mValues.begin()->isMinValue(),
                         "First offset is expected to be zero!");

        std::vector<Domain*>::const_iterator itOffsets = offsets.begin() + 1;
        for (; itOffsets != offsets.end(); ++itOffsets)
        {
            std::vector<const Domain*> nextLevelResult;
            std::vector<const Domain*>::const_iterator itItems = result.begin();
            for (; itItems != result.end(); ++itItems)
            {
                std::vector<Domain*> items;
                const Array::Interface &array =
                    dynCast<const Array::Interface&>(**itItems);

                items = array.getItem(**itOffsets);
                nextLevelResult.insert(nextLevelResult.end(),
                                       items.begin(),
                                       items.end());
            }

            result.swap(nextLevelResult);
        }
    }

    return result;
}


Domain *
Pointer::dereferenceAndMerge(const State &state) const
{
    Domain *mergedValue = NULL;
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        if (it->second->mType != Target::Block)
            continue;

        const Domain *source = state.findBlock(*it->second->mTarget);
        CANAL_ASSERT(source);

        std::vector<const Domain*> values =
            dereference(source, it->second->mOffsets);

        std::vector<const Domain*>::const_iterator it = values.begin();
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

Pointer *
Pointer::bitcast(const llvm::Type &type) const
{
    return new Pointer(*this, type);
}

Pointer *
Pointer::getElementPtr(const std::vector<Domain*> &offsets,
                       const llvm::Type &type,
                       const Constructors &constructors) const
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

    Pointer *result = new Pointer(*this, type);

    // TODO: handle mNumericOffset.

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
                Domain *newLast = constructors.createInteger(64);
                newLast->add(*targetOffsets.back(), **offsets.begin());
                delete targetOffsets.back();
                targetOffsets.pop_back();
                targetOffsets.push_back(newLast);
                continue;
            }

            targetOffsets.push_back((*offsetIt)->clone());
        }
    }

    // Delete the offsets, because this method takes ownership of them
    // and it no longer needs them.
    std::for_each(offsets.begin(), offsets.end(), llvm::deleter<Domain>);

    return result;
}

void
Pointer::store(const Domain &value, State &state) const
{
    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        if (it->second->mType != Target::Block)
            continue;

        const Domain *source = state.findBlock(*it->second->mTarget);
        CANAL_ASSERT(source);

        Domain *result = source->clone();
        std::vector<Domain*> destinations = dereference(result,
                                                        it->second->mOffsets);

        std::vector<Domain*>::iterator itDest = destinations.begin();
        for (; itDest != destinations.end(); ++itDest)
            (*itDest)->merge(value);

        if (state.hasGlobalBlock(*it->second->mTarget))
            state.addGlobalBlock(*it->second->mTarget, result);
        else
            state.addFunctionBlock(*it->second->mTarget, result);
    }
}

bool
Pointer::isSingleTarget() const
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

Pointer *
Pointer::clone() const
{
    return new Pointer(*this);
}

bool
Pointer::operator==(const Domain &value) const
{
    // Check if the value has the same type.
    const Pointer *pointer =
        dynCast<const Pointer*>(&value);

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

void
Pointer::merge(const Domain &value)
{
    const Pointer &vv = dynCast<const Pointer&>(value);
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
Pointer::memoryUsage() const
{
    size_t size = sizeof(Pointer);

    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        size += it->second->memoryUsage();

    return size;
}

std::string
Pointer::toString() const
{
    StringStream ss;
    ss << "pointer\n";
    ss << "    type " << Canal::toString(mType) << "\n";

    PlaceTargetMap::const_iterator it = mTargets.begin(),
        itend = mTargets.end();

    for (; it != itend; ++it)
        ss << indent(it->second->toString(mEnvironment.getSlotTracker()), 4);

    return ss.str();
}

void
Pointer::setZero(const llvm::Value *place)
{
    mTargets.clear();
    addTarget(Target::Constant,
              place,
              NULL,
              std::vector<Domain*>(),
              NULL);
}

} // namespace Pointer
} // namespace Canal
