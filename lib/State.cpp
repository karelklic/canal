#include "State.h"
#include "Domain.h"
#include "Utils.h"
#include "Environment.h"
#include "SlotTracker.h"

namespace Canal {

State::State() : mReturnedValue(NULL)
{
}

State::State(const State &state)
    : mGlobalVariables(state.mGlobalVariables),
      mGlobalBlocks(state.mGlobalBlocks),
      mFunctionVariables(state.mFunctionVariables),
      mFunctionBlocks(state.mFunctionBlocks),
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
    return mGlobalVariables.size() == state.mGlobalVariables.size() &&
        mGlobalBlocks.size() == state.mGlobalBlocks.size() &&
        mFunctionVariables.size() == state.mFunctionVariables.size() &&
        mFunctionBlocks.size() == state.mFunctionBlocks.size() &&
        mGlobalVariables == state.mGlobalVariables &&
        mGlobalBlocks == state.mGlobalBlocks &&
        mFunctionVariables == state.mFunctionVariables &&
        mFunctionBlocks == state.mFunctionBlocks &&
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
State::merge(const State &state)
{
    mFunctionVariables.merge(state.mFunctionVariables);
    mFunctionBlocks.merge(state.mFunctionBlocks);
    mergeGlobal(state);
    mergeReturnedValue(state);
    mVariableArguments.merge(state.mVariableArguments);
}

void
State::mergeGlobal(const State &state)
{
    mGlobalVariables.merge(state.mGlobalVariables);
    mGlobalBlocks.merge(state.mGlobalBlocks);
}

void
State::mergeReturnedValue(const State &state)
{
    if (!state.mReturnedValue)
        return;

    if (mReturnedValue)
        mReturnedValue->join(*state.mReturnedValue);
    else
        mReturnedValue = state.mReturnedValue->clone();
}

void
State::mergeFunctionBlocks(const State &state)
{
    mFunctionBlocks.merge(state.mFunctionBlocks);
}

static bool
containsPlace(const llvm::BasicBlock &basicBlock,
              const llvm::Value *place)
{
    llvm::BasicBlock::const_iterator it = basicBlock.begin(),
        itend = basicBlock.end();

    for (; it != itend; ++it)
    {
        if (it == place)
            return true;
    }

    return false;
}

static bool
containsPlace(const llvm::Function &function,
              const llvm::Value *place)
{
    llvm::Function::const_iterator it = function.begin(),
        itend = function.end();

    for (; it != itend; ++it)
    {
        if (containsPlace(*it, place))
            return true;
    }

    return false;
}

void
State::mergeForeignFunctionBlocks(const State &state,
                                  const llvm::Function &currentFunction)
{
    // Merge function blocks that do not belong to current function.
    StateMap::const_iterator it2 = state.mFunctionBlocks.begin(),
        it2end = state.mFunctionBlocks.end();

    for (; it2 != it2end; ++it2)
    {
	StateMap::iterator it1 = mFunctionBlocks.find(it2->first);
	if (it1 == mFunctionBlocks.end())
        {
            if (containsPlace(currentFunction, it2->first))
                continue;

            mFunctionBlocks.insert(*it2);
        }
	else if (*it1->second != *it2->second)
            it1->second.mutable_()->join(*it2->second);
    }
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
State::addGlobalBlock(const llvm::Value &place, Domain *value)
{
    mGlobalBlocks.insert(place, value);
}

void
State::addFunctionBlock(const llvm::Value &place, Domain *value)
{
    mFunctionBlocks.insert(place, value);
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
    StateMap::const_iterator it = mGlobalVariables.find(&place);
    if (it != mGlobalVariables.end())
        return it->second.data();

    it = mFunctionVariables.find(&place);
    if (it != mFunctionVariables.end())
        return it->second.data();

    return NULL;
}

const Domain *
State::findBlock(const llvm::Value &place) const
{
    StateMap::const_iterator it = mGlobalBlocks.find(&place);
    if (it != mGlobalBlocks.end())
        return it->second.data();

    it = mFunctionBlocks.find(&place);
    if (it != mFunctionBlocks.end())
        return it->second.data();

    return NULL;
}

bool
State::hasGlobalBlock(const llvm::Value &place) const
{
    return (mGlobalBlocks.find(&place) != mGlobalBlocks.end());
}

size_t
State::memoryUsage() const
{
    size_t result = sizeof(State);
    result += mGlobalVariables.memoryUsage();
    result += mGlobalBlocks.memoryUsage();
    result += mFunctionVariables.memoryUsage();
    result += mFunctionBlocks.memoryUsage();

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

} // namespace Canal
