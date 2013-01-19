#include "Pointer.h"
#include "ArraySingleItem.h"
#include "Utils.h"
#include "State.h"
#include "SlotTracker.h"
#include "ProductVector.h"
#include "IntegerSet.h"
#include "IntegerUtils.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Pointer {

Pointer::Pointer(const Environment &environment,
                 const llvm::PointerType &type)
    : Domain(environment, Domain::PointerKind),
      mTop(false),
      mType(type)
{
}

Pointer::Pointer(const Pointer &value)
    : Domain(value),
      mTargets(value.mTargets),
      mTop(value.mTop),
      mType(value.mType)
{
    PlaceTargetMap::iterator it = mTargets.begin(), itend = mTargets.end();
    for (; it != itend; ++it)
        it->second = new Target(*it->second);
}

Pointer::Pointer(const Pointer &value,
                 const llvm::PointerType &newType)
    : Domain(value),
      mTargets(value.mTargets),
      mTop(value.mTop),
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
    if (mTop)
        return;

    CANAL_ASSERT_MSG(place,
                     "Place is mandatory.");

    CANAL_ASSERT_MSG(llvm::isa<llvm::Instruction>(place) ||
                     llvm::isa<llvm::GlobalValue>(place) ||
                     llvm::isa<llvm::Constant>(place),
                     "Place must be either an instruction or a global value or a constant.");

    Target *pointerTarget = new Target(mEnvironment,
                                       type,
                                       target,
                                       offsets,
                                       numericOffset);

    PlaceTargetMap::iterator it = mTargets.find(place);
    if (it != mTargets.end())
    {
        it->second->join(*pointerTarget);
        delete pointerTarget;
    }
    else
        mTargets.insert(PlaceTargetMap::value_type(place, pointerTarget));
}

Domain *
Pointer::dereferenceAndMerge(const State &state) const
{
    if (mTop)
    {
        const llvm::Type &elementType = *mType.getElementType();
        Domain *result = mEnvironment.getConstructors().create(elementType);
        result->setTop();
        return result;
    }

    Domain *mergedValue = NULL;
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        if (it->second->mType != Target::Block)
            continue;

        const Domain *source = state.findBlock(*it->second->mTarget);
        CANAL_ASSERT(source);

        std::vector<Domain*> offsets;
        if (!it->second->mOffsets.empty())
        {
            CANAL_ASSERT_MSG(Integer::Utils::getSet(*it->second->mOffsets[0]).mValues.size() == 1,
                             "First offset is expected to be zero!");

            CANAL_ASSERT_MSG(Integer::Utils::getSet(*it->second->mOffsets[0]).mValues.begin()->isMinValue(),
                             "First offset is expected to be zero!");

            offsets = std::vector<Domain*>(it->second->mOffsets.begin() + 1,
                                           it->second->mOffsets.end());
        }

        Domain *loaded = source->load(*getValueType().getElementType(),
                                      offsets);

        if (mergedValue)
        {
            mergedValue->join(*loaded);
            delete loaded;
        }
        else
            mergedValue = loaded;
    }

    return mergedValue;
}

Pointer *
Pointer::bitcast(const llvm::PointerType &type) const
{
    return new Pointer(*this, type);
}

Pointer *
Pointer::getElementPtr(const std::vector<Domain*> &offsets,
                       const llvm::PointerType &type,
                       const Constructors &constructors) const
{
    CANAL_ASSERT_MSG(!offsets.empty(),
                     "getElementPtr must be called with some offsets.");

    // Check that all offsets are 64-bit integers.
    std::vector<Domain*>::const_iterator offsetIt = offsets.begin();
    for (; offsetIt != offsets.end(); ++offsetIt)
    {
        CANAL_ASSERT_MSG(Integer::Utils::getBitWidth(**offsetIt) == 64,
                         "GetElementPtr offsets must have 64 bits!");
    }

    Pointer *result = new Pointer(*this, type);

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
    CANAL_ASSERT(!mTop);

    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
    PlaceTargetMap::const_iterator it = mTargets.begin(),
        itend = mTargets.end();

    for (; it != itend; ++it)
    {
        if (it->second->mType != Target::Block)
            continue;

        const Domain *source = state.findBlock(*it->second->mTarget);
        CANAL_ASSERT(source);

        std::vector<Domain*> offsets;
        if (!it->second->mOffsets.empty())
        {
            CANAL_ASSERT_MSG(Integer::Utils::getSet(*it->second->mOffsets[0]).mValues.size() == 1,
                             "First offset is expected to be zero!");

            CANAL_ASSERT_MSG(Integer::Utils::getSet(*it->second->mOffsets[0]).mValues.begin()->isMinValue(),
                             "First offset is expected to be zero!");

            offsets = std::vector<Domain*>(it->second->mOffsets.begin() + 1,
                                           it->second->mOffsets.end());
        }

        Domain *result = source->clone();
        result->store(value, offsets, mTargets.size() == 1);

        if (state.hasGlobalBlock(*it->second->mTarget))
            state.addGlobalBlock(*it->second->mTarget, result);
        else
            state.addFunctionBlock(*it->second->mTarget, result);
    }
}

bool
Pointer::isConstant() const
{
    if (mTop || mTargets.size() != 1)
        return false;

    const Target *target = mTargets.begin()->second;

    if (target->mNumericOffset &&
        !Integer::Utils::isConstant(*target->mNumericOffset))
    {
        return false;
    }

    std::vector<Domain*>::const_iterator it = target->mOffsets.begin();
    for (; it != target->mOffsets.end(); ++it)
    {
        if (!Integer::Utils::isConstant(**it))
            return false;
    }

    return true;
}

Pointer *
Pointer::clone() const
{
    return new Pointer(*this);
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
    if (mTop)
        ss << "pointer top\n";
    else
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
    llvm::DeleteContainerSeconds(mTargets);
    addTarget(Target::Constant,
              place,
              NULL,
              std::vector<Domain*>(),
              NULL);
}

bool
Pointer::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    // Check if the value has the same type.
    const Pointer &pointer = checkedCast<Pointer>(value);
    if (&pointer.mType != &mType)
        return false;

    if (pointer.mTop != mTop)
        return false;

    // Check if it has the same number of targets.
    if (pointer.mTargets.size() != mTargets.size())
        return false;

    // Check the targets.
    PlaceTargetMap::const_iterator it1 = pointer.mTargets.begin(),
        it2 = mTargets.begin();

    for (; it2 != mTargets.end(); ++it1, ++it2)
    {
        if (it1->first != it2->first || *it1->second != *it2->second)
            return false;
    }

    return true;
}

bool
Pointer::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

Pointer &
Pointer::join(const Domain &value)
{
    if (isTop())
        return *this;

    if (value.isBottom())
        return *this;

    if (value.isTop())
    {
        setTop();
        return *this;
    }

    const Pointer &vv = llvm::cast<Pointer>(value);
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
            it->second->join(*valueit->second);
    }

    return *this;
}

Pointer &
Pointer::meet(const Domain &value)
{
    if (isBottom())
        return *this;

    if (value.isTop())
        return *this;

    if (value.isBottom())
    {
        setBottom();
        return *this;
    }

    const Pointer &vv = llvm::cast<Pointer>(value);
    CANAL_ASSERT_MSG(&vv.mType == &mType,
                     "Unexpected different types in a pointer merge ("
                     << Canal::toString(vv.mType) << " != "
                     << Canal::toString(mType) << ")");

    if (isTop())
    {
        mTop = false;
        CANAL_ASSERT(mTargets.empty());
    }

    PlaceTargetMap::iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
    {
        PlaceTargetMap::const_iterator valueit = vv.mTargets.find(it->first);
        if (it == vv.mTargets.end())
        {
            delete it->second;
            mTargets.erase(it);
        }
        else
            it->second->meet(*valueit->second);
    }

    return *this;
}

bool
Pointer::isBottom() const
{
    return !mTop && mTargets.empty();
}

void
Pointer::setBottom()
{
    mTop = false;
    llvm::DeleteContainerSeconds(mTargets);
}

bool
Pointer::isTop() const
{
    return mTop;
}

void
Pointer::setTop()
{
    mTop = true;
    llvm::DeleteContainerSeconds(mTargets);
}

} // namespace Pointer
} // namespace Canal
