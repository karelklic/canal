#include "PointerTarget.h"
#include "ArrayInterface.h"
#include "IntegerContainer.h"
#include "SlotTracker.h"
#include "State.h"
#include "Utils.h"
#include "Domain.h"
#include <sstream>
#include <llvm/BasicBlock.h>
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
      mInstruction(target),
      mOffsets(offsets),
      mNumericOffset(numericOffset)
{
    CANAL_ASSERT_MSG(type == FunctionBlock ||
                     type == FunctionVariable ||
                     type == GlobalBlock ||
                     type == GlobalVariable ||
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
                                       mInstruction(target.mInstruction),
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
    case FunctionBlock:
    case FunctionVariable:
    case GlobalBlock:
    case GlobalVariable:
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
    case FunctionBlock:
    case FunctionVariable:
    case GlobalBlock:
    case GlobalVariable:
    {
        CANAL_ASSERT(mInstruction == target.mInstruction);
        CANAL_ASSERT(mOffsets.size() == target.mOffsets.size());
        std::vector<Domain*>::iterator it1 = mOffsets.begin();
        std::vector<Domain*>::const_iterator it2 = target.mOffsets.begin();
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
        break;
    case FunctionBlock:
    case FunctionVariable:
    {
        const llvm::Instruction &instruction =
            llvmCast<llvm::Instruction>(*mInstruction);

        slotTracker.setActiveFunction(*instruction.getParent()->getParent());
        std::string name(Canal::getName(instruction, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";

        switch (mType)
        {
        case FunctionBlock:    ss << " %^"; break;
        case FunctionVariable: ss << " %";  break;
        case GlobalBlock:      ss << " @^"; break;
        default:               CANAL_DIE();
        }

        ss << name;
        break;
    }
    case GlobalBlock:
    {
        const llvm::Instruction *instruction =
            llvm::dyn_cast<llvm::Instruction>(mInstruction);

        if (instruction)
            slotTracker.setActiveFunction(*instruction->getParent()->getParent());
        else
            CANAL_ASSERT_MSG(llvm::isa<llvm::GlobalVariable>(mInstruction),
                             "Unexpected type of pointer assignment source");

        std::string name(Canal::getName(*mInstruction, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";

        ss << " @^" << name;
        break;
    }
    case GlobalVariable:
    {
        std::string name(Canal::getName(*mInstruction, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";
        ss << " @" << name;
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
    std::vector<Domain*> result;

    switch (mType)
    {
    case Constant:
        CANAL_DIE();
    case FunctionBlock:
    case FunctionVariable:
    case GlobalBlock:
    case GlobalVariable:
    {
        result.push_back(state.findBlock(*mInstruction));
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
    default:
        CANAL_DIE();
    }
}

} // namespace Pointer
} // namespace Canal
