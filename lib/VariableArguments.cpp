#include "VariableArguments.h"

VariableArguments::VariableArguments(const VariableArguments &arguments)
    : mCalls(arguments.mCalls)
{
    // Clone all values.
    CallMap::const_iterator it1 = mCalls.begin();
    for (; it1 != mCalls.end(); ++it1)
    {
        std::vector<Domain*>::const_iterator it2 = it1->second->begin();
        for (; it2 != it1->second->end(); ++it2)
            *it2 = (*it2)->clone();
    }
}

VariableArguments::~VariableArguments()
{
    // Delete all values.
    CallMap::const_iterator it1 = mCalls.begin();
    for (; it1 != mCalls.end(); ++it1)
    {
        std::vector<Domain*>::const_iterator it2 = it1->second->begin();
        for (; it2 != it1->second->end(); ++it2)
            delete *it2;
    }
}

void
VariableArguments::merge(const VariableArguments &arguments)
{
    // Merge all values.
}

void
VariableArguments::addArgument(const llvm::Instruction &place,
                               Domain *argument)
{
}
