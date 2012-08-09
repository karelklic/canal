#include "Utils.h"
#include "SlotTracker.h"
#include <llvm/ADT/APInt.h>
#include <llvm/Value.h>
#include <llvm/Instruction.h>
#include <llvm/BasicBlock.h>
#include <llvm/Type.h>
#include <llvm/Constant.h>
#include <sstream>
#include <execinfo.h>
#include <cxxabi.h>

namespace Canal {

std::string
toString(const llvm::APInt &num)
{
    std::stringstream ss;
    ss << "0x" << num.toString(16, /*signed=*/false);
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
toString(const llvm::Type &type)
{
    std::string s;
    llvm::raw_string_ostream os(s);
    os << type;
    os.flush();
    return s;
}

std::string
toString(const llvm::Constant &constant)
{
    std::string s;
    llvm::raw_string_ostream os(s);
    os << constant;
    os.flush();
    return s;
}

std::string
indent(const std::string &input, int spaces)
{
    return std::string(spaces, ' ') + indentExceptFirstLine(input, spaces);
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
    if (value.hasName())
        ss << value.getName().data();
    else
    {
        int id;
        if (isGlobal)
            id = slotTracker.getGlobalSlot(value);
        else
        {
            const llvm::Instruction &inst =
                llvm::cast<const llvm::Instruction>(value);

            slotTracker.setActiveFunction(*inst.getParent()->getParent());
            id = slotTracker.getLocalSlot(value);
        }
        if (id >= 0)
            ss << id;
        else
            return "";
    }
    return ss.str();
}

std::string
getCurrentBacktrace()
{
    // http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
    std::stringstream result;
    void *array[1024];
    int size = backtrace(array, 1024);
    char **messages = backtrace_symbols(array, size);

    // Skip the first stack frame (points here).
    for (int i = 1; i < size && messages; ++i)
    {
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        // Find parantheses and +address offset surrounding mangled
        // name.
        for (char *p = messages[i]; *p; ++p)
        {
            if (*p == '(')
                mangled_name = p;
            else if (*p == '+')
                offset_begin = p;
            else if (*p == ')')
            {
                offset_end = p;
                break;
            }
        }

        // If the line could be processed, attempt to demangle the
        // symbol.  Otherwise, print the whole line.
        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin)
        {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            int status;
            char *real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
            // If demangling is successful, output the demangled
            // function name.  Otherwise, output the mangled function
            // name.
            if (status == 0)
            {
                result << "[bt]: (" << i << ") " << messages[i] << " : "
                       << real_name << "+" << offset_begin << offset_end
                       << std::endl;
            }
            else
            {
                result << "[bt]: (" << i << ") " << messages[i] << " : "
                       << mangled_name << "+" << offset_begin << offset_end
                       << std::endl;
            }
            free(real_name);
        }
        else
            result << "[bt]: (" << i << ") " << messages[i] << std::endl;
    }

    free(messages);
    return result.str();
}

} // namespace Canal
