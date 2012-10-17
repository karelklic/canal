#include "PointerTarget.h"
#include "ArrayInterface.h"
#include "IntegerContainer.h"
#include "SlotTracker.h"
#include "State.h"
#include "Utils.h"
#include "Domain.h"
#include <sstream>
#include <llvm/BasicBlock.h>
#include <llvm/Function.h>
#include <llvm/ADT/APInt.h>

namespace Canal {
namespace Pointer {

Target::Target(const Environment &environment,
               Type type,
               const llvm::Value *target,
               const std::vector<Domain*> &offsets,
               Domain *numericOffset)
    : mEnvironment(environment),
      mType(type),
      mTarget(target),
      mOffsets(offsets),
      mNumericOffset(numericOffset)
{
    CANAL_ASSERT_MSG(type == Block ||
                     type == Function ||
                     type == Constant,
                     "Invalid type.");

    CANAL_ASSERT_MSG((type == Constant && !target) ||
                     (type != Constant && target),
                     "Invalid value of target.");

    CANAL_ASSERT_MSG(type != Constant || offsets.empty(),
                     "Offsets cannot be present for constant pointers "
                     "as the semantics is not defined.");
}

Target::Target(const Target &target) : mEnvironment(target.mEnvironment),
                                       mType(target.mType),
                                       mTarget(target.mTarget),
                                       mOffsets(target.mOffsets),
                                       mNumericOffset(target.mNumericOffset)
{
    std::vector<Domain*>::iterator it = mOffsets.begin();
    for (; it != mOffsets.end(); ++it)
        *it = (*it)->clone();

    if (mNumericOffset)
        mNumericOffset = mNumericOffset->clone();
}

Target::~Target()
{
    std::vector<Domain*>::iterator it = mOffsets.begin(),
        itend = mOffsets.end();

    for (; it != itend; ++it)
        delete *it;

    delete mNumericOffset;
}

bool
Target::operator==(const Target &target) const
{
    if (mType != target.mType)
        return false;

    if ((!mNumericOffset && target.mNumericOffset) ||
        (mNumericOffset && !target.mNumericOffset) ||
        (mNumericOffset && *mNumericOffset != *target.mNumericOffset))
    {
        return false;
    }

    switch (mType)
    {
    case Constant:
        // We already compared numeric offsets.
        break;
    case Block:
    {
        if (mOffsets.size() != target.mOffsets.size())
            return false;

        // Check the targets.
        std::vector<Domain*>::const_iterator it1 = mOffsets.begin(),
            it2 = target.mOffsets.begin();
        for (; it1 != mOffsets.end(); ++it1, ++it2)
        {
            if (**it1 != **it2)
                return false;
        }

        return mTarget == target.mTarget;
    }
    case Function:
        return mTarget == target.mTarget;
    default:
        CANAL_DIE();
    }

    return true;
}

bool
Target::operator!=(const Target &target) const
{
    return !(*this == target);
}

void
Target::merge(const Target &target)
{
    // Merge numeric offsets.
    if (!mNumericOffset && target.mNumericOffset)
        mNumericOffset = target.mNumericOffset->clone();
    else if (mNumericOffset && !target.mNumericOffset)
    {
        const Integer::Container &numericOffsetInt =
            dynCast<const Integer::Container&>(*mNumericOffset);

        llvm::APInt zero = llvm::APInt::getNullValue(
            numericOffsetInt.getBitWidth());
        Integer::Container zeroContainer(mEnvironment, zero);
        mNumericOffset->merge(zeroContainer);
    }
    else if (mNumericOffset)
        mNumericOffset->merge(*target.mNumericOffset);

    CANAL_ASSERT(mType == target.mType);
    switch (mType)
    {
    case Constant:
        // We already merged numeric offsets.
        break;
    case Block:
    {
        CANAL_ASSERT(mTarget == target.mTarget);
        CANAL_ASSERT_MSG(mOffsets.size() == target.mOffsets.size(),
                         "Expected equal number of offsets, but got "
                         << mOffsets.size() << " and "
                         << target.mOffsets.size());

        std::vector<Domain*>::iterator it1 = mOffsets.begin();
        std::vector<Domain*>::const_iterator it2 = target.mOffsets.begin();
        for (; it1 != mOffsets.end(); ++it1, ++it2)
            (*it1)->merge(**it2);

        break;
    }
    case Function:
        CANAL_ASSERT(mTarget == target.mTarget);
        break;
    default:
        CANAL_DIE();
    }
}

size_t
Target::memoryUsage() const
{
    size_t size = sizeof(Target);

    // Add the size of the offsets.
    std::vector<Domain*>::const_iterator it = mOffsets.begin();
    for (; it != mOffsets.end(); ++it)
        size += (*it)->memoryUsage();

    // Add the size of the numeric offset.
    if (mNumericOffset)
        size += mNumericOffset->memoryUsage();

    return size;
}

std::string
Target::toString(SlotTracker &slotTracker) const
{
    std::stringstream ss;
    ss << "target";

    switch (mType)
    {
    case Constant:
        ss << " constant";
        break;
    case Block:
    {
        const llvm::Instruction *instruction =
            llvmCast<llvm::Instruction>(mTarget);

        if (instruction)
        {
            const llvm::Function &function =
                *instruction->getParent()->getParent();

            ss << " @" << Canal::getName(function, slotTracker);
            ss << ":^" << Canal::getName(*instruction, slotTracker);
        }
        else
            ss << " ^" << Canal::getName(*mTarget, slotTracker);

        break;
    }
    case Function:
    {
        const llvm::Function &function =
            llvmCast<llvm::Function>(*mTarget);

        ss << " @" << Canal::getName(function, slotTracker);
        break;
    }
    default:
        CANAL_DIE();
    }

    ss << std::endl;

    if (!mOffsets.empty())
    {
        ss << "    offsets" << std::endl;
        std::vector<Domain*>::const_iterator it = mOffsets.begin();
        for (; it != mOffsets.end(); ++it)
            ss << indent((*it)->toString(), 8);
    }

    if (mNumericOffset)
    {
        ss << "    numericOffset" << std::endl;
        ss << indent(mNumericOffset->toString(), 8);
    }

    return ss.str();
}

std::vector<Domain*>
Target::dereference(const State &state) const
{
    CANAL_ASSERT_MSG(mType == Block,
                     "Only Block pointer targets can be dereferenced.");

    std::vector<Domain*> result;
    result.push_back(state.findBlock(*mTarget));
    CANAL_ASSERT(result[0]);

    std::vector<Domain*>::const_iterator itOffsets = mOffsets.begin();
    for (; itOffsets != mOffsets.end(); ++itOffsets)
    {
        std::vector<Domain*> nextLevelResult;
        std::vector<Domain*>::const_iterator itItems = result.begin();
        for (; itItems != result.end(); ++itItems)
        {
            std::vector<Domain*> items;
            Array::Interface &array = dynCast<Array::Interface&>(**itItems);
            items = array.getItem(**itOffsets);
            nextLevelResult.insert(nextLevelResult.end(),
                                   items.begin(),
                                   items.end());
        }

        result.swap(nextLevelResult);
    }

    return result;
}

} // namespace Pointer
} // namespace Canal
