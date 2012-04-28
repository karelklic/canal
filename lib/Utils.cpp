#include "Utils.h"
#include "SlotTracker.h"
#include <llvm/ADT/APInt.h>
#include <llvm/Value.h>
#include <sstream>

namespace Canal {

std::string
toString(const llvm::APInt &num)
{
    std::stringstream ss;
    ss << num.toString(16, /*signed=*/false);
    std::string unsignedNum = num.toString(10, false);
    std::string signedNum = num.toString(10, true);
    if (unsignedNum == signedNum)
        ss << " (" << signedNum << ")";
    else
    {
        ss << " (unsigned " << unsignedNum;
        ss << ", signed " << signedNum;
        ss << ")";
    }
    return ss.str();
}

std::string
indentExceptFirstLine(const std::string &input, int spaces)
{
    std::string space(spaces, ' ');
    std::string result = input;
    size_t found = result.find_first_of("\n");
    while (found != std::string::npos && found < result.size() - 1)
    {
        result.insert(found + 1, space);
        found = result.find_first_of("\n", found + 1 + space.size());
    }
    return result;
}

std::string
getName(const llvm::Value &value, SlotTracker &slotTracker)
{
    bool isGlobal = llvm::isa<llvm::GlobalValue>(value);
    std::stringstream ss;
    ss << (isGlobal ? "@" : "%");
    if (value.hasName())
        ss << value.getName().data();
    else
    {
        int id;
        if (isGlobal)
            id = slotTracker.getGlobalSlot(value);
        else
            id = slotTracker.getLocalSlot(value);
        if (id >= 0)
            ss << id;
        else
            return "";
    }
    return ss.str();
}

} // namespace Canal
