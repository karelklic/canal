#include "PointerTarget.h"
#include "ArrayInterface.h"
#include "IntegerContainer.h"
#include "SlotTracker.h"
#include "State.h"
#include "Utils.h"
#include "Value.h"
#include <sstream>
#include <llvm/BasicBlock.h>

namespace Canal {
namespace Pointer {

Target::Target() : mType(Target::Uninitialized),
                   mNumericOffset(NULL)
{
}

Target::Target(const Target &target) : mType(target.mType),
                                       mConstant(target.mConstant),
                                       mInstruction(target.mInstruction),
                                       mOffsets(target.mOffsets),
                                       mNumericOffset(target.mNumericOffset)
{
    std::vector<Value*>::iterator it = mOffsets.begin(),
        itend = mOffsets.end();

    for (; it != itend; ++it)
        *it = (*it)->clone();

    if (mNumericOffset)
        mNumericOffset = mNumericOffset->clone();
}

Target::~Target()
{
    std::vector<Value*>::iterator it = mOffsets.begin(),
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
    case Uninitialized:
        return true;
    case Constant:
        return mConstant == target.mConstant;
    case MemoryBlock:
    {
        if (mOffsets.size() != target.mOffsets.size())
            return false;

        // Check the targets.
        std::vector<Value*>::const_iterator it1 = mOffsets.begin(),
            it2 = target.mOffsets.begin();
        for (; it1 != mOffsets.end(); ++it1, ++it2)
        {
            if (*it1 != *it2)
                return false;
        }

        return mInstruction == target.mInstruction;
    }
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
    CANAL_ASSERT(mType == target.mType);
    switch (mType)
    {
    case Uninitialized:
        break;
    case Constant:
        // TODO: mConstant can be abstract value.
        CANAL_ASSERT(mConstant == target.mConstant);
        break;
    case MemoryBlock:
    {
        CANAL_ASSERT(mInstruction == target.mInstruction);
        CANAL_ASSERT(mOffsets.size() == target.mOffsets.size());

        // Merge numeric offsets.
        if (!mNumericOffset && target.mNumericOffset)
            mNumericOffset = target.mNumericOffset->clone();
        else if (mNumericOffset && !target.mNumericOffset)
        {
            const Integer::Container &numericOffsetInt =
                dynamic_cast<const Integer::Container&>(*mNumericOffset);

            llvm::APInt zero = llvm::APInt::getNullValue(numericOffsetInt.getBitWidth());
            mNumericOffset->merge(Integer::Container(zero));
        }
        else if (mNumericOffset)
            mNumericOffset->merge(*target.mNumericOffset);

        std::vector<Value*>::iterator it1 = mOffsets.begin();
        std::vector<Value*>::const_iterator it2 = target.mOffsets.begin();
        for (; it1 != mOffsets.end(); ++it1, ++it2)
            (*it1)->merge(**it2);
        break;
    }
    default:
        CANAL_DIE();
    }
}

size_t
Target::memoryUsage() const
{
    size_t offsetSize = 0;
    std::vector<Value*>::const_iterator it = mOffsets.begin();
    for (; it != mOffsets.end(); ++it)
        offsetSize += (*it)->memoryUsage();

    return sizeof(Target) + offsetSize;
}

std::string
Target::toString(const State *state, SlotTracker &slotTracker) const
{
    std::stringstream ss;
    ss << "Pointer::Target: ";
    if (!mOffsets.empty())
    {
        ss << "{" << std::endl;
        std::vector<Value*>::const_iterator it = mOffsets.begin();
        for (; it != mOffsets.end(); ++it)
            ss << "    offset: " << indentExceptFirstLine((*it)->toString(state), 18) << std::endl;
        ss << "    target: ";
    }

    switch (mType)
    {
    case Uninitialized:
        ss << "uninitialized";
        break;
    case Constant:
        ss << mConstant;
        break;
    case MemoryBlock:
    {
        const llvm::Instruction &instruction =
            llvm::cast<llvm::Instruction>(*mInstruction);

        slotTracker.setActiveFunction(*instruction.getParent()->getParent());
        std::string name(Canal::getName(instruction, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";

        ss << name;
        if (state)
        {
            Value *block = state->findBlock(*mInstruction);
            ss << " ";
            int indentation = (mOffsets.empty() ? 17 : 12) + 1 + name.length();
            ss << (block ? indentExceptFirstLine(block->toString(state), indentation) : "<error: block missing in current state!>");
        }
        break;
    }
    default:
        CANAL_DIE();
    }

    if (!mOffsets.empty())
        ss << std::endl << "}";

    return ss.str();
}

std::vector<Value*>
Target::dereference(const State &state) const
{
    std::vector<Value*> result;

    switch (mType)
    {
    case Uninitialized:
    case Constant:
        CANAL_DIE();
    case MemoryBlock:
    {
        result.push_back(state.findBlock(*mInstruction));
        CANAL_ASSERT(result[0]);

        std::vector<Value*>::const_iterator itOffsets = mOffsets.begin();
        for (; itOffsets != mOffsets.end(); ++itOffsets)
        {
            std::vector<Value*> nextLevelResult;
            std::vector<Value*>::const_iterator itItems = result.begin();
            for (; itItems != result.end(); ++itItems)
            {
                std::vector<Value*> items;
                Array::Interface &array = dynamic_cast<Array::Interface&>(**itItems);
                items = array.getItem(**itOffsets);
                nextLevelResult.insert(nextLevelResult.end(),
                                       items.begin(),
                                       items.end());
            }

            result.swap(nextLevelResult);
        }

        return result;
    }
    default:
        CANAL_DIE();
    }
}

} // namespace Pointer
} // namespace Canal
