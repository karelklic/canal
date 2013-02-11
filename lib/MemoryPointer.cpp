#include "MemoryPointer.h"
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
namespace Memory {

Pointer::Pointer(const Environment &environment,
                 const llvm::PointerType &type)
    : Domain(environment, Domain::PointerKind),
      mTop(false),
      mNumericOffset(NULL),
      mType(type)
{
}

Pointer::Pointer(const Pointer &value)
    : Domain(value),
      mTop(value.mTop),
      mBlockTargets(value.mBlockTargets),
      mNumericOffset(value.mNumericOffset),
      mFunctionTargets(value.mFunctionTargets),
      mType(value.mType)
{
    // Clone the block targets so they are not shared.
    std::map<const llvm::Value*, Domain*>::iterator
        it = mBlockTargets.begin(),
        itend = mBlockTargets.end();

    for (; it != itend; ++it)
        it->second = it->second->clone();

    // Clone the numeric offset.
    if (mNumericOffset)
        mNumericOffset = mNumericOffset->clone();
}

Pointer::Pointer(const Pointer &value,
                 const llvm::PointerType &newType)
    : Domain(value),
      mTop(value.mTop),
      mBlockTargets(value.mBlockTargets),
      mNumericOffset(value.mNumericOffset),
      mFunctionTargets(value.mFunctionTargets),
      mType(newType)
{
    // Clone the block targets so they are not shared.
    std::map<const llvm::Value*, Domain*>::iterator
        it = mBlockTargets.begin(),
        itend = mBlockTargets.end();

    for (; it != itend; ++it)
        it->second = it->second->clone();

    // Clone the numeric offset.
    if (mNumericOffset)
        mNumericOffset = mNumericOffset->clone();
}

Pointer::~Pointer()
{
    llvm::DeleteContainerSeconds(mBlockTargets);
    delete mNumericOffset;
}

void
Pointer::addTarget(const Domain &numericOffset)
{
    if (mTop)
        return;

    if (mNumericOffset)
        mNumericOffset->join(numericOffset);
    else
        mNumericOffset = numericOffset.clone();
}

void
Pointer::addTarget(const llvm::Value &target, Domain *offset)
{
    if (mTop)
    {
        delete offset;
        return;
    }

    if (!offset)
    {
        llvm::APInt concreteZero(/*numBits=*/64, /*val=*/0L);
        offset = mEnvironment.getConstructors().createInteger(concreteZero);
    }

    std::map<const llvm::Value*, Domain*>::iterator
        it = mBlockTargets.find(&target);

    if (it != mBlockTargets.end())
    {
        it->second->join(*offset);
        delete offset;
    }
    else
        mBlockTargets.insert(std::map<const llvm::Value*, Domain*>::value_type(&target, offset));
}

void
Pointer::addTarget(const llvm::Function &function)
{
    if (mTop)
        return;

    mFunctionTargets.insert(&function);
}

Domain *
Pointer::dereferenceAndMerge(const State &state) const
{
    CANAL_ASSERT(mFunctionTargets.empty());
    Domain *result = NULL;

    if (mTop)
    {
        const llvm::Type &elementType = *mType.getElementType();
        result = mEnvironment.getConstructors().create(elementType);
        result->setTop();
        return result;
    }

    std::map<const llvm::Value*, Domain*>::const_iterator
        it = mBlockTargets.begin(),
        itend = mBlockTargets.end();

    for (; it != itend; ++it)
    {
        const Domain *source = state.findBlock(*it->first);
        CANAL_ASSERT_MSG(source,
                         "Unable to find referenced memory block: "
                         << *it->first);

        Domain *loaded = NULL;
        CANAL_ASSERT(it->second);
        loaded = source->load(*getValueType().getElementType(),
                              *it->second);

        if (result)
        {
            result->join(*loaded);
            delete loaded;
        }
        else
            result = loaded;
    }

    return result;
}

Pointer *
Pointer::withOffset(const Domain &offset,
                    const llvm::PointerType &type) const
{
    CANAL_ASSERT(mFunctionTargets.empty());

    Pointer *result = new Pointer(*this, type);

    // Adjust the numeric offset by the provided offset.
    if (result->mNumericOffset)
    {
        Domain *newOffset = mEnvironment.getConstructors().createInteger(64);
        newOffset->add(*result->mNumericOffset, offset);
        delete result->mNumericOffset;
        result->mNumericOffset = newOffset;
    }

    // Iterate over all targets, and adjust the target offsets.
    std::map<const llvm::Value*, Domain*>::iterator
        it = result->mBlockTargets.begin(),
        itend = result->mBlockTargets.end();

    for (; it != itend; ++it)
    {
        Domain *newOffset = mEnvironment.getConstructors().createInteger(64);
        newOffset->add(*it->second, offset);
        delete it->second;
        it->second = newOffset;
    }

    return result;
}

void
Pointer::store(const Domain &value, State &state) const
{
    CANAL_ASSERT(mFunctionTargets.empty());
    CANAL_ASSERT(!mTop);

    // Go through all target memory blocks for the pointer and merge
    // them with the value being stored.
    std::map<const llvm::Value*, Domain*>::const_iterator
        it = mBlockTargets.begin(),
        itend = mBlockTargets.end();

    for (; it != itend; ++it)
    {
        const Domain *source = state.findBlock(*it->first);
        CANAL_ASSERT_MSG(source,
                         "Unable to find referenced memory block: "
                         << *it->first);

        Domain *result = source->clone();
        result->store(value, *it->second, mBlockTargets.size() == 1);

        if (state.hasGlobalBlock(*it->first))
            state.addGlobalBlock(*it->first, result);
        else
            state.addFunctionBlock(*it->first, result);
    }
}

bool
Pointer::isConstant() const
{
    if (isTop() || isBottom())
        return false;

    if (!mBlockTargets.empty() && mNumericOffset)
        return false;

    if (!mNumericOffset &&
        mFunctionTargets.empty() &&
        mBlockTargets.size() != 1)
    {
        return false;
    }

    if (mNumericOffset && !Integer::Utils::isConstant(*mNumericOffset))
        return false;

    if (mFunctionTargets.size() > 1)
        return false;

    if (!mBlockTargets.empty())
    {
        if (!Integer::Utils::isConstant(*mBlockTargets.begin()->second))
            return false;
    }

    return true;
}

void
Pointer::widen(const Pointer &second)
{
    std::map<const llvm::Value*, Domain*>::iterator
        it = mBlockTargets.begin(),
        itend = mBlockTargets.end();

    for (; it != itend; ++it)
        it->second->setTop();

    if (mNumericOffset)
        mNumericOffset->setTop();
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

    // Add the size of mBlockTargets map.
    size += mBlockTargets.size() *
        sizeof(std::map<const llvm::Value*, Domain*>::value_type);

    std::map<const llvm::Value*, Domain*>::const_iterator
        it = mBlockTargets.begin(),
        itend = mBlockTargets.end();

    for (; it != itend; ++it)
        size += it->second->memoryUsage();

    // Add the size of the numeric offset.
    if (mNumericOffset)
        size += mNumericOffset->memoryUsage();

    // Add the size of the function targets.
    size += mFunctionTargets.size() * sizeof(std::set<const llvm::Function*>::value_type);

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

    if (!mBlockTargets.empty())
    {
        ss << "    blockTargets\n";

        std::map<const llvm::Value*, Domain*>::const_iterator
            it = mBlockTargets.begin(),
            itend = mBlockTargets.end();

        for (; it != itend; ++it)
        {
            StringStream target;
            const llvm::Instruction *instruction =
                dynCast<llvm::Instruction>(it->first);

            if (instruction)
            {
                const llvm::Function *function =
                    instruction->getParent()->getParent();

                if (function)
                    target << "@" << Canal::getName(*function, mEnvironment.getSlotTracker()) << ":";

                target << "^" << Canal::getName(*instruction, mEnvironment.getSlotTracker());
            }
            else
                target << "^" << Canal::getName(*it->first, mEnvironment.getSlotTracker());

            target << "\n";
            target << indent(it->second->toString(), 4);

            ss << indent(target.str(), 8);
        }
    }

    if (mNumericOffset)
    {
        ss << "    numericOffset = "
           << indentExceptFirstLine(mNumericOffset->toString(),
                                    strlen("    numericOffset = "));
    }

    if (!mFunctionTargets.empty())
    {
        ss << "    functionTargets\n";

        std::set<const llvm::Function*>::const_iterator
            it = mFunctionTargets.begin(),
            itend = mFunctionTargets.end();

        for (; it != itend; ++it)
        {
            const llvm::Function &function =
                checkedCast<llvm::Function>(**it);

            ss << "        @" << Canal::getName(function, mEnvironment.getSlotTracker()) << "\n";
        }
    }

    return ss.str();
}

void
Pointer::setZero(const llvm::Value *place)
{
    CANAL_ASSERT(!mNumericOffset);
    CANAL_ASSERT(!mTop);
    CANAL_ASSERT(mBlockTargets.empty());
    CANAL_ASSERT(mFunctionTargets.empty());

    llvm::APInt concreteZero(/*numBits=*/64, /*val=*/0L);
    mNumericOffset = mEnvironment.getConstructors().createInteger(concreteZero);
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

    // Check if it has the same number of block targets.
    if (pointer.mBlockTargets.size() != mBlockTargets.size())
        return false;

    // Check the block targets.
    std::map<const llvm::Value*, Domain*>::const_iterator
        it1 = pointer.mBlockTargets.begin(),
        it2 = mBlockTargets.begin(),
        it2end = mBlockTargets.end();

    for (; it2 != it2end; ++it1, ++it2)
    {
        if (it1->first != it2->first || *it1->second != *it2->second)
            return false;
    }

    // Check the numeric offset.
    if (bool(mNumericOffset) != bool(pointer.mNumericOffset))
        return false;

    if (mNumericOffset && *mNumericOffset != *pointer.mNumericOffset)
        return false;

    // Check the function targets.
    if (mFunctionTargets != pointer.mFunctionTargets)
        return false;

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

    const Pointer &pointer = checkedCast<Pointer>(value);
    CANAL_ASSERT_MSG(&pointer.mType == &mType,
                     "Unexpected different types in a pointer join ("
                     << Canal::toString(pointer.mType) << " != "
                     << Canal::toString(mType) << ")");

    // Join the block targets.
    std::map<const llvm::Value*, Domain*>::const_iterator
        it1 = pointer.mBlockTargets.begin(),
        it1end = pointer.mBlockTargets.end();

    for (; it1 != it1end; ++it1)
    {
        std::map<const llvm::Value*, Domain*>::iterator
            it2 = mBlockTargets.find(it1->first);

        if (it2 == mBlockTargets.end())
        {
            mBlockTargets.insert(
                std::map<const llvm::Value*, Domain*>::value_type(
                    it1->first, it1->second->clone()));
        }
        else
            it2->second->join(*it1->second);
    }

    // Join the numeric offset.
    if (pointer.mNumericOffset)
    {
        if (mNumericOffset)
            mNumericOffset->join(*pointer.mNumericOffset);
        else
            mNumericOffset = pointer.mNumericOffset->clone();
    }

    // Join the function targets.
    mFunctionTargets.insert(pointer.mFunctionTargets.begin(),
                            pointer.mFunctionTargets.end());

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

    const Pointer &pointer = llvm::cast<Pointer>(value);
    CANAL_ASSERT_MSG(&pointer.mType == &mType,
                     "Unexpected different types in a pointer meet ("
                     << Canal::toString(pointer.mType) << " != "
                     << Canal::toString(mType) << ")");

    if (isTop())
    {
        mTop = false;
        CANAL_ASSERT(mBlockTargets.empty());
        CANAL_ASSERT(!mNumericOffset);
        CANAL_ASSERT(mFunctionTargets.empty());
    }

    // Meet the block targets.
    std::map<const llvm::Value*, Domain*>::iterator
        it1 = mBlockTargets.begin(),
        it1end = mBlockTargets.end();

    for (; it1 != it1end; ++it1)
    {
        std::map<const llvm::Value*, Domain*>::const_iterator
            it2 = pointer.mBlockTargets.find(it1->first);

        if (it2 == pointer.mBlockTargets.end())
        {
            delete it1->second;
            mBlockTargets.erase(it1);
        }
        else
            it1->second->meet(*it2->second);
    }

    // Meet the numeric offset.
    if (mNumericOffset)
    {
        if (pointer.mNumericOffset)
            mNumericOffset->meet(*pointer.mNumericOffset);
        else
        {
            delete mNumericOffset;
            mNumericOffset = NULL;
        }
    }

    // Meet the function targets.
    std::set<const llvm::Function*> intersection;  // Destination of intersect
    std::set_intersection(mFunctionTargets.begin(),
                          mFunctionTargets.end(),
                          pointer.mFunctionTargets.begin(),
                          pointer.mFunctionTargets.end(),
                          std::inserter(intersection, intersection.end()));

    mFunctionTargets.swap(intersection);
    return *this;
}

bool
Pointer::isBottom() const
{
    return !mTop &&
        mBlockTargets.empty() &&
        !mNumericOffset &&
        mFunctionTargets.empty();
}

void
Pointer::setBottom()
{
    mTop = false;
    llvm::DeleteContainerSeconds(mBlockTargets);
    delete mNumericOffset;
    mNumericOffset = NULL;
    mFunctionTargets.clear();
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
    llvm::DeleteContainerSeconds(mBlockTargets);
    delete mNumericOffset;
    mNumericOffset = NULL;
    mFunctionTargets.clear();
}

} // namespace Memory
} // namespace Canal
