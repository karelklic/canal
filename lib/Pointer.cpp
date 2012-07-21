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

InclusionBased::InclusionBased(const llvm::Module &module)
    : mModule(module), mBitcastFrom(NULL), mBitcastTo(NULL)
{
}

InclusionBased *
InclusionBased::clone() const
{
    return new InclusionBased(*this);
}

bool
InclusionBased::operator==(const Value &value) const
{
    // Check if the value has the same type.
    const InclusionBased *pointer = dynamic_cast<const InclusionBased*>(&value);
    if (!pointer)
        return false;

    if (pointer->mBitcastFrom != mBitcastFrom ||
        pointer->mBitcastTo != mBitcastTo)
        return false;

    // Check if it has the same number of targets.
    if (pointer->mTargets.size() != mTargets.size())
        return false;

    // Check the targets.
    PlaceTargetMap::const_iterator it1 = pointer->mTargets.begin(),
        it2 = mTargets.begin();
    for (; it2 != mTargets.end(); ++it1, ++it2)
    {
        if (it1->first != it2->first || it1->second != it2->second)
            return false;
    }

    return true;
}

void
InclusionBased::merge(const Value &value)
{
    if (const Constant *constant = dynamic_cast<const Constant*>(&value))
    {
        CANAL_ASSERT(constant->isGetElementPtr());
        // TODO: get the pointer from the constant.
        CANAL_NOT_IMPLEMENTED();
        return;
    }

    const InclusionBased &vv = dynamic_cast<const InclusionBased&>(value);
    CANAL_ASSERT_MSG(vv.mBitcastFrom == mBitcastFrom &&
                     vv.mBitcastTo == mBitcastTo,
                     "Unexpected different bitcasts in a pointer merge");

    PlaceTargetMap::const_iterator valueit = vv.mTargets.begin();
    for (; valueit != vv.mTargets.end(); ++valueit)
    {
        PlaceTargetMap::iterator it = mTargets.find(valueit->first);
        if (it == mTargets.end())
            mTargets.insert(*valueit);
        else
            it->second.merge(valueit->second);
    }
}

size_t
InclusionBased::memoryUsage() const
{
    size_t size = sizeof(InclusionBased);
    PlaceTargetMap::const_iterator it = mTargets.begin();
    for (; it != mTargets.end(); ++it)
        size += it->second.memoryUsage();
    return size;
}

std::string
InclusionBased::toString(const State *state) const
{
    SlotTracker slotTracker(mModule);
    std::stringstream ss;
    ss << "Pointer::InclusionBased: [" << std::endl;
    for (PlaceTargetMap::const_iterator it = mTargets.begin(); it != mTargets.end(); ++it)
    {
        const llvm::Instruction &instruction = llvm::cast<llvm::Instruction>(*it->first);
        slotTracker.setActiveFunction(*instruction.getParent()->getParent());
        std::string name(Canal::getName(*it->first, slotTracker));
        if (name.empty())
            name = "<failed to name the location>";

        ss << "    { assigned: " << name << std::endl;
        ss << "      target: " << indentExceptFirstLine(it->second.toString(state, slotTracker), 14) << " }" << std::endl;
    }

    if (mBitcastFrom)
    {
        std::string s;
        llvm::raw_string_ostream os(s);
        os << "    bitcast from: " << *mBitcastFrom;
        os.flush();
        ss << s << std::endl;
    }

    if (mBitcastTo)
    {
        std::string s;
        llvm::raw_string_ostream os(s);
        os << "    bitcast to: " << *mBitcastTo;
        os.flush();
        ss << s << std::endl;
    }

    ss << "]";
    return ss.str();
}

void
InclusionBased::addConstantTarget(const llvm::Value *instruction,
                                  size_t constant)
{
    Target newTarget;
    newTarget.mType = Target::Constant;
    newTarget.mConstant = constant;

    PlaceTargetMap::iterator it = mTargets.find(instruction);
    if (it != mTargets.end())
        it->second.merge(newTarget);
    else
        mTargets.insert(PlaceTargetMap::value_type(instruction, newTarget));
}

void
InclusionBased::addMemoryTarget(const llvm::Value *instruction,
                                const llvm::Value *target)
{
    Target newTarget;
    newTarget.mType = Target::MemoryBlock;
    newTarget.mInstruction = target;

    PlaceTargetMap::iterator it = mTargets.find(instruction);
    if (it != mTargets.end())
        it->second.merge(newTarget);
    else
        mTargets.insert(PlaceTargetMap::value_type(instruction, newTarget));
}

} // namespace Pointer
} // namespace Canal
