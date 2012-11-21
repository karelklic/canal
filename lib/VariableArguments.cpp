#include "VariableArguments.h"
#include "Domain.h"
#include "Utils.h"

namespace Canal {

static void
cloneDomains(std::vector<Domain*> &domains)
{
    std::vector<Domain*>::iterator it = domains.begin();
    for (; it != domains.end(); ++it)
        *it = (*it)->clone();
}

VariableArguments::VariableArguments(const VariableArguments &arguments)
    : mCalls(arguments.mCalls)
{
    // Clone all values.
    CallMap::iterator it = mCalls.begin();
    for (; it != mCalls.end(); ++it)
        cloneDomains(it->second);
}

VariableArguments::~VariableArguments()
{
    // Delete all values.
    CallMap::iterator it = mCalls.begin();
    for (; it != mCalls.end(); ++it)
        llvm::DeleteContainerPointers(it->second);
}

static bool
equal(const std::vector<Domain*> &first, const std::vector<Domain*> &second)
{
    if (first.size() != second.size())
        return false;

    std::vector<Domain*>::const_iterator it1 = first.begin();
    std::vector<Domain*>::const_iterator it2 = second.begin();
    for (; it1 != first.end(); ++it1, ++it2)
    {
        if (**it1 != **it2)
            return false;
    }

    return true;
}

bool
VariableArguments::operator==(const VariableArguments &arguments) const
{
    if (mCalls.size() != arguments.mCalls.size())
        return false;

    CallMap::const_iterator it1 = mCalls.begin();
    CallMap::const_iterator it2 = arguments.mCalls.begin();
    for (; it1 != mCalls.end(); ++it1, ++it2)
    {
        if (it1->first != it2->first ||
            !equal(it1->second, it2->second))
        {
            return false;
        }
    }

    return true;
}

static void
mergeDomains(std::vector<Domain*> &first, const std::vector<Domain*> &second)
{
    CANAL_ASSERT_MSG(first.size() == second.size(),
                     "Argument lists must have the same length.");

    std::vector<Domain*>::iterator it1 = first.begin();
    std::vector<Domain*>::const_iterator it2 = second.begin();
    for (; it1 != first.end(); ++it1, ++it2)
        (*it1)->merge(**it2);
}

void
VariableArguments::merge(const VariableArguments &arguments)
{
    // Merge all values.
    CallMap::const_iterator it2 = arguments.mCalls.begin(),
        it2end = arguments.mCalls.end();

    for (; it2 != it2end; ++it2)
    {
	CallMap::iterator it1 = mCalls.find(it2->first);
	if (it1 == mCalls.end())
        {
            std::vector<Domain*> arguments = it2->second;
            cloneDomains(arguments);
            mCalls.insert(CallMap::value_type(it2->first,
                                              arguments));
        }
	else
            mergeDomains(it1->second, it2->second);
    }
}

void
VariableArguments::addArgument(const llvm::Instruction &place,
                               Domain *argument)
{
    CallMap::iterator it = mCalls.find(&place);
    if (it == mCalls.end())
    {
        std::vector<Domain*> arguments;
        arguments.push_back(argument);
        mCalls.insert(CallMap::value_type(&place,
                                          arguments));
    }
    else
        it->second.push_back(argument);
}

} // namespace Canal
