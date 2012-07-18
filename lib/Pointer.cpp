//
// This file is distributed under the University of Illinois Open
// Source License. See the LICENSE file for details.
//
#include "Pointer.h"
#include "ArraySingleItem.h"
#include "Constant.h"
#include "Utils.h"
#include "State.h"
#include "SlotTracker.h"
#include "Structure.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/BasicBlock.h>
#include <algorithm>
#include <sstream>
#include <iostream>

namespace Canal {
namespace Pointer {

Target::Target() : mType(Target::Uninitialized)
{
}

Target::Target(const Target &target) : mType(target.mType),
                                       mConstant(target.mConstant),
                                       mInstruction(target.mInstruction),
                                       mOffsets(target.mOffsets)
{
    std::vector<Value*>::iterator it = mOffsets.begin(),
        itend = mOffsets.end();

    for (; it != itend; ++it)
        *it = (*it)->clone();
}

Target::~Target()
{
    std::vector<Value*>::iterator it = mOffsets.begin(),
        itend = mOffsets.end();
    for (; it != itend; ++it)
        delete *it;
}

bool
Target::operator==(const Target &target) const
{
    if (mType != target.mType)
        return false;

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
        const llvm::Instruction &instruction = llvm::cast<llvm::Instruction>(*mInstruction);
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
                if (Array::Array *array = dynamic_cast<Array::Array*>(*itItems))
                    items = array->getItems(**itOffsets);
                else if (Structure *structure = dynamic_cast<Structure*>(structure))
                    items = structure->getItems(**itOffsets);
                else
                    CANAL_DIE();

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
