#include "OperationalState.h"
#include "AbstractValue.h"

namespace Operational {

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

    FunctionVariables = rhs.FunctionVariables;
    VariablesMap::iterator vit = FunctionVariables.begin(),
        vitend = FunctionVariables.end();
    for (; vit != vitend; ++vit)
        vit->second = vit->second->clone();

    GlobalVariables = rhs.GlobalVariables;
    vit = GlobalVariables.begin();
    vitend = GlobalVariables.end();
    for (; vit != vitend; ++vit)
        vit->second = vit->second->clone();

    GlobalBlocks = rhs.GlobalBlocks;
    MemoryBlockList::iterator it = GlobalBlocks.begin(), itend = GlobalBlocks.end();
    for (; it != itend; ++it)
        *it = (*it)->clone();

    FunctionBlocks = rhs.FunctionBlocks;
    it = FunctionBlocks.begin();
    itend = FunctionBlocks.end();
    for (; it != itend; ++it)
        *it = (*it)->clone();

    return *this;
}

bool State::operator==(const State &rhs) const
{
    return true;
}

bool State::operator!=(const State &rhs) const
{
    return !operator==(rhs);
}

void State::clear()
{
    // Delete stack variables.
    VariablesMap::const_iterator vit = FunctionVariables.begin(),
        vitend = FunctionVariables.end();
    for (; vit != vitend; ++vit)
        delete vit->second;
    FunctionVariables.clear();

    // Delete global variables.
    vit = GlobalVariables.begin();
    vitend = GlobalVariables.end();
    for (; vit != vitend; ++vit)
        delete vit->second;
    GlobalVariables.clear();

    // Delete heap blocks.
    MemoryBlockList::const_iterator it = GlobalBlocks.begin(),
        itend = GlobalBlocks.end();
    for (; it != itend; ++it)
        delete *it;
    GlobalBlocks.clear();

    // Delete stack blocks.
    it = FunctionBlocks.begin();
    itend = FunctionBlocks.end();
    for (; it != itend; ++it)
        delete *it;
    FunctionBlocks.clear();
}

void State::merge(const State &State)
{
    // Merge stack variables.
    VariablesMap::const_iterator vit = State.FunctionVariables.begin(),
        vitend = State.FunctionVariables.end();

    for (; vit != vitend; ++vit)
    {
	VariablesMap::iterator it = FunctionVariables.find(vit->first);
	if (it == FunctionVariables.end())
            FunctionVariables[vit->first] = vit->second;
	else
            it->second->merge(*vit->second);
    }

    // Merge global variables.
    vit = State.GlobalVariables.begin();
    vitend = State.GlobalVariables.end();
    for (; vit != vitend; ++vit)
    {
	VariablesMap::iterator it = GlobalVariables.find(vit->first);
	if (it == GlobalVariables.end())
            GlobalVariables[vit->first] = vit->second;
	else
            it->second->merge(*vit->second);
    }

    // GlobalBlocks and FunctionBlocks are merged through pointers in
    // FunctionVariables and GlobalVariables.
}

} // namespace Operational
