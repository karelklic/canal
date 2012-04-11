#include "State.h"
#include "Value.h"
#include <llvm/Support/raw_ostream.h>

namespace Canal {

State::State()
{
}

State::State(const State &rhs)
{
    this->operator=(rhs);
}

State::~State()
{
    clear();
}

State &State::operator=(const State &rhs)
{
    clear();

    mFunctionVariables = rhs.mFunctionVariables;
    VariablesMap::iterator vit = mFunctionVariables.begin(),
        vitend = mFunctionVariables.end();
    for (; vit != vitend; ++vit)
    {
        vit->second = vit->second->clone();

        StateValue *stateValue = dynamic_cast<StateValue*>(vit->second);
        if (stateValue)
            stateValue->setState(this);
    }

    mGlobalVariables = rhs.mGlobalVariables;
    vit = mGlobalVariables.begin();
    vitend = mGlobalVariables.end();
    for (; vit != vitend; ++vit)
    {
        vit->second = vit->second->clone();

        StateValue *stateValue = dynamic_cast<StateValue*>(vit->second);
        if (stateValue)
            stateValue->setState(this);
    }

    mGlobalBlocks = rhs.mGlobalBlocks;
    MemoryBlockList::iterator it = mGlobalBlocks.begin(),
        itend = mGlobalBlocks.end();
    for (; it != itend; ++it)
    {
        *it = (*it)->clone();

        StateValue *stateValue = dynamic_cast<StateValue*>(*it);
        if (stateValue)
            stateValue->setState(this);
    }

    mFunctionBlocks = rhs.mFunctionBlocks;
    it = mFunctionBlocks.begin();
    itend = mFunctionBlocks.end();
    for (; it != itend; ++it)
    {
        *it = (*it)->clone();

        StateValue *stateValue = dynamic_cast<StateValue*>(*it);
        if (stateValue)
            stateValue->setState(this);
    }

    return *this;
}

bool State::operator==(const State &rhs) const
{
    // Quickly compare sizes.
    if (mGlobalVariables.size() != rhs.mGlobalVariables.size())
        return false;
    if (mFunctionVariables.size() != rhs.mFunctionVariables.size())
        return false;

    // Compare global variables.
    VariablesMap::const_iterator vit = rhs.mGlobalVariables.begin(),
        vitend = rhs.mGlobalVariables.end();

    for (; vit != vitend; ++vit)
    {
	VariablesMap::const_iterator it = mGlobalVariables.find(vit->first);
	if (it == mGlobalVariables.end())
            return false;
	else if (*it->second != *vit->second)
            return false;
    }

    // Compare stack variables.
    vit = rhs.mFunctionVariables.begin();
    vitend = rhs.mFunctionVariables.end();

    for (; vit != vitend; ++vit)
    {
	VariablesMap::const_iterator it = mFunctionVariables.find(vit->first);
	if (it == mFunctionVariables.end())
            return false;
	else if (*it->second != *vit->second)
            return false;
    }

    return true;
}

bool State::operator!=(const State &rhs) const
{
    return !operator==(rhs);
}

void State::clear()
{
    // Delete stack variables.
    VariablesMap::const_iterator vit = mFunctionVariables.begin(),
        vitend = mFunctionVariables.end();
    for (; vit != vitend; ++vit)
        delete vit->second;
    mFunctionVariables.clear();

    // Delete global variables.
    vit = mGlobalVariables.begin();
    vitend = mGlobalVariables.end();
    for (; vit != vitend; ++vit)
        delete vit->second;
    mGlobalVariables.clear();

    // Delete heap blocks.
    MemoryBlockList::const_iterator it = mGlobalBlocks.begin(),
        itend = mGlobalBlocks.end();
    for (; it != itend; ++it)
        delete *it;
    mGlobalBlocks.clear();

    // Delete stack blocks.
    it = mFunctionBlocks.begin();
    itend = mFunctionBlocks.end();
    for (; it != itend; ++it)
        delete *it;
    mFunctionBlocks.clear();
}

void State::merge(const State &state)
{
    // Merge stack variables.
    VariablesMap::const_iterator vit = state.mFunctionVariables.begin(),
        vitend = state.mFunctionVariables.end();

    for (; vit != vitend; ++vit)
    {
	VariablesMap::iterator it = mFunctionVariables.find(vit->first);
	if (it == mFunctionVariables.end())
            mFunctionVariables[vit->first] = vit->second->clone();
	else
            it->second->merge(*vit->second);
    }

    // Merge global variables.
    vit = state.mGlobalVariables.begin();
    vitend = state.mGlobalVariables.end();
    for (; vit != vitend; ++vit)
    {
	VariablesMap::iterator it = mGlobalVariables.find(vit->first);
	if (it == mGlobalVariables.end())
            mGlobalVariables[vit->first] = vit->second->clone();
	else
            it->second->merge(*vit->second);
    }

    // mGlobalBlocks and mFunctionBlocks are merged through pointers in
    // mFunctionVariables and mGlobalVariables.
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
			      const State &state)
{
    ostream << "OperationalState(function variables: "
            << state.mFunctionVariables.size()
            << ", global variables: "
            << state.mGlobalVariables.size()
            << ")";
}

} // namespace Canal
