#include "State.h"
#include "Value.h"
#include <llvm/Support/raw_ostream.h>

namespace Canal {

State::State()
{
}

State::State(const State &state)
{
    operator=(state);
}

State::~State()
{
    clear();
}

static void copyMap(PlaceValueMap &destination, const PlaceValueMap &source)
{
    destination = source;
    PlaceValueMap::iterator it = destination.begin(),
        itend = destination.end();
    for (; it != itend; ++it)
        it->second = it->second->clone();
}

State &State::operator=(const State &state)
{
    clear();
    copyMap(mFunctionVariables, state.mFunctionVariables);
    copyMap(mFunctionBlocks, state.mFunctionBlocks);
    copyMap(mGlobalVariables, state.mGlobalVariables);
    copyMap(mGlobalBlocks, state.mGlobalBlocks);
    return *this;
}

static bool equalMaps(const PlaceValueMap &map1, const PlaceValueMap &map2)
{
    PlaceValueMap::const_iterator it2 = map2.begin(),
        it2end = map2.end();

    for (; it2 != it2end; ++it2)
    {
	PlaceValueMap::const_iterator it1 = map1.find(it2->first);
	if (it1 == map1.end())
            return false;
	else if (*it1->second != *it2->second)
            return false;
    }
    return true;
}

bool State::operator==(const State &state) const
{
    // Quickly compare sizes.
    if (mGlobalVariables.size() != state.mGlobalVariables.size())
        return false;
    if (mGlobalBlocks.size() != state.mGlobalBlocks.size())
        return false;
    if (mFunctionVariables.size() != state.mFunctionVariables.size())
        return false;
    if (mFunctionBlocks.size() != state.mFunctionBlocks.size())
        return false;

    if (!equalMaps(mGlobalVariables, state.mGlobalVariables))
        return false;
    if (!equalMaps(mGlobalBlocks, state.mGlobalBlocks))
        return false;
    if (!equalMaps(mFunctionVariables, state.mFunctionVariables))
        return false;
    if (!equalMaps(mFunctionBlocks, state.mFunctionBlocks))
        return false;

    return true;
}

bool State::operator!=(const State &state) const
{
    return !operator==(state);
}

static void clearMap(PlaceValueMap &map)
{
    PlaceValueMap::const_iterator it = map.begin(),
        itend = map.end();
    for (; it != itend; ++it)
        delete it->second;
    map.clear();
}

void State::clear()
{
    clearMap(mFunctionVariables);
    clearMap(mFunctionBlocks);
    clearMap(mGlobalVariables);
    clearMap(mGlobalBlocks);
}

// Merges map2 into map1.
static void mergeMaps(PlaceValueMap &map1, const PlaceValueMap &map2)
{
    PlaceValueMap::const_iterator it2 = map2.begin(),
        it2end = map2.end();

    for (; it2 != it2end; ++it2)
    {
	PlaceValueMap::iterator it1 = map1.find(it2->first);
	if (it1 == map1.end())
            map1.insert(PlaceValueMap::value_type(it2->first, it2->second->clone()));
	else
            it1->second->merge(*it2->second);
    }
}

void State::merge(const State &state)
{
    mergeMaps(mFunctionVariables, state.mFunctionVariables);
    mergeMaps(mFunctionBlocks, state.mFunctionBlocks);
    mergeMaps(mGlobalVariables, state.mGlobalVariables);
    mergeMaps(mGlobalBlocks, state.mGlobalBlocks);
}

static void replaceOrInsertMapItem(PlaceValueMap &map, const llvm::Value *instruction, Value *value)
{
    PlaceValueMap::iterator it = map.find(instruction);
    if (it != map.end())
    {
        delete it->second;
        it->second = value;
    }
    else
        map.insert(std::pair<const llvm::Value*, Value*>(instruction, value));
}

void State::addGlobalVariable(const llvm::Value *instruction, Value *value)
{
    replaceOrInsertMapItem(mGlobalVariables, instruction, value);
}

void State::addFunctionVariable(const llvm::Value *instruction, Value *value)
{
    replaceOrInsertMapItem(mFunctionVariables, instruction, value);
}

void State::addGlobalBlock(const llvm::Value *instruction, Value *value)
{
    replaceOrInsertMapItem(mGlobalBlocks, instruction, value);
}

void State::addFunctionBlock(const llvm::Value *instruction, Value *value)
{
    replaceOrInsertMapItem(mFunctionBlocks, instruction, value);
}

Value *State::findVariable(const llvm::Value *place) const
{
    PlaceValueMap::const_iterator it = mGlobalVariables.find(place);
    if (it != mGlobalVariables.end())
        return it->second;

    it = mFunctionVariables.find(place);
    if (it != mFunctionVariables.end())
        return it->second;

    return NULL;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
			      const State &state)
{
    ostream << "State(function variables: "
            << state.getFunctionVariables().size()
            << ", global variables: "
            << state.getGlobalVariables().size()
            << ")";
}

} // namespace Canal
