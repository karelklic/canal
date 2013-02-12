#include "MemoryState.h"
#include "Domain.h"
#include "Utils.h"
#include "Environment.h"
#include "SlotTracker.h"

namespace Canal {
namespace Memory {

State::State() : mReturnedValue(NULL)
{
}

State::State(const State &state)
    : mBlocks(state.mBlocks),
      mVariables(state.mVariables),
      mReturnedValue(state.mReturnedValue),
      mVariableArguments(state.mVariableArguments)
{
    if (mReturnedValue)
        mReturnedValue = mReturnedValue->clone();
}

State::~State()
{
    delete mReturnedValue;
}

bool
State::operator==(const State &state) const
{
    if (&state == this)
        return true;

    // Quickly compare sizes.
    return mVariables.size() == state.mVariables.size() &&
        mBlocks.size() == state.mBlocks.size() &&
        mVariables == state.mVariables &&
        mBlocks == state.mBlocks &&
        ((mReturnedValue && state.mReturnedValue &&
          *mReturnedValue == *state.mReturnedValue) ||
         (!mReturnedValue && !state.mReturnedValue)) &&
        mVariableArguments == state.mVariableArguments;
}

bool
State::operator!=(const State &state) const
{
    return !operator==(state);
}

void
State::join(const State &state)
{
    mVariables.join(state.mVariables);
    mBlocks.join(state.mBlocks);
    joinReturnedValue(state);
    mVariableArguments.join(state.mVariableArguments);
}

void
State::joinGlobal(const State &state)
{
    mVariables.joinGlobals(state.mVariables);
    mBlocks.joinHeap(state.mBlocks);
}

void
State::joinReturnedValue(const State &state)
{
    if (!state.mReturnedValue)
        return;

    if (mReturnedValue)
        mReturnedValue->join(*state.mReturnedValue);
    else
        mReturnedValue = state.mReturnedValue->clone();
}

void
State::joinStackBlocks(const State &state)
{
    mBlocks.joinStack(state.mBlocks);
}

void
State::joinForeignStackBlocks(const State &state,
                              const llvm::Function &currentFunction)
{
    mBlocks.joinForeignStack(state.mBlocks,
                             currentFunction);
}

void State::addGlobalVariable(const llvm::Value &place, Domain *value)
{
    mGlobalVariables.insert(place, value);
}

void
State::addFunctionVariable(const llvm::Value &place, Domain *value)
{
    mFunctionVariables.insert(place, value);
}

void
State::setReturnedValue(Domain *value)
{
    CANAL_ASSERT(!mReturnedValue);
    mReturnedValue = value;
}

void
State::mergeToReturnedValue(const Domain &value)
{
    if (mReturnedValue)
        mReturnedValue->join(value);
    else
        mReturnedValue = value.clone();
}

void
State::addVariableArgument(const llvm::Instruction &place, Domain *argument)
{
    mVariableArguments.addArgument(place, argument);
}

const Domain *
State::findVariable(const llvm::Value &place) const
{
    VariableMap::const_iterator it = mVariables.find(&place);
    if (it != mVariables.end())
        return it->second.data();

    return NULL;
}

const Block *
State::findBlock(const llvm::Value &place) const
{
    BlockMap::const_iterator it = mBlocks.find(&place);
    if (it != mBlocks.end())
        return it->second.data();

    return NULL;
}

SharedDataPointer<Block>
State::findBlock(const llvm::Value &place)
{
    BlockMap::iterator it = mBlocks.find(&place);
    if (it != mBlocks.end())
        return it->second;

    return SharedDataPointer<Block>();
}

size_t
State::memoryUsage() const
{
    size_t result = sizeof(State);
    result += mVariables.memoryUsage();
    result += mBlocks.memoryUsage();
    if (mReturnedValue)
        result += mReturnedValue->memoryUsage();

    return result;
}

std::string
State::toString(const llvm::Value &place,
                SlotTracker &slotTracker) const
{
    StringStream ss, name;
    if (place.hasName())
        name << place.getName();
    else
    {
        if (llvm::isa<llvm::Instruction>(place))
        {
            const llvm::Instruction &instruction =
                checkedCast<llvm::Instruction>(place);

            slotTracker.setActiveFunction(
                *instruction.getParent()->getParent());

            name << slotTracker.getLocalSlot(place);
        }
        else
            name << slotTracker.getGlobalSlot(place);
    }

    StateMap::const_iterator it = mFunctionVariables.find(&place);
    if (it != mFunctionVariables.end())
    {
        ss << "%" << name.str() << " = "
           << Canal::indentExceptFirstLine(it->second->toString(),
                                           name.str().length() + 4);
    }

    it = mFunctionBlocks.find(&place);
    if (it != mFunctionBlocks.end())
    {
        ss << "%^" << name.str() << " = "
           << Canal::indentExceptFirstLine(it->second->toString(),
                                           name.str().length() + 5);
    }

    if (ss.str().empty() &&
        llvm::isa<llvm::Instruction>(place))
    {
        ss << "%" << name.str() << " = undefined\n";
    }

    it = mGlobalVariables.find(&place);
    if (it != mGlobalVariables.end())
    {
        ss << "@" << name.str() << " = "
           << Canal::indentExceptFirstLine(it->second->toString(),
                                           name.str().length() + 4);
    }

    it = mGlobalBlocks.find(&place);
    if (it != mGlobalBlocks.end())
    {
        ss << "@^" << name.str() << " = "
           << Canal::indentExceptFirstLine(it->second->toString(),
                                           name.str().length() + 5);
    }

    if (ss.str().empty())
        ss << "@" << name.str() << " = undefined\n";

    return ss.str();
}

} // namespace Memory
} // namespace Canal
