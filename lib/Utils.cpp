#include "Utils.h"
#include "SlotTracker.h"
#include <execinfo.h>
#include <cxxabi.h>

namespace Canal {

std::string
toString(const llvm::APInt &num)
{
    StringStream ss;
    ss << "0x" << num.toString(16, /*signed=*/false);
    std::string unsignedNum = num.toString(10, false);
    std::string signedNum = num.toString(10, true);

    // Print decimal number only when signed and unsigned
    // representation differ or the representations are the same but
    // nontrivial.
    if (unsignedNum == signedNum)
    {
        if (signedNum.length() > 1)
            ss << " (" << signedNum << ")";
    }
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
    StringStream ss;
    ss << const_cast<llvm::Type&>(type);
    return ss.str();
}

std::string
indent(const std::string &input, int spaces)
{
    return std::string(spaces, ' ') +
        indentExceptFirstLine(input, spaces);
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
    StringStream ss;

    if (value.hasName())
        ss << value.getName();
    else if (llvm::isa<llvm::GlobalValue>(value))
    {
        int id = slotTracker.getGlobalSlot(value);
        if (id > 0)
            ss << id;
    }
    else if (llvm::isa<llvm::BasicBlock>(value))
    {
        const llvm::BasicBlock &block =
            checkedCast<llvm::BasicBlock>(value);

        slotTracker.setActiveFunction(*block.getParent());
        ss << "<label>:" << slotTracker.getLocalSlot(value);
    }
    else
    {
        const llvm::Instruction &inst =
            checkedCast<llvm::Instruction>(value);

        slotTracker.setActiveFunction(
            *inst.getParent()->getParent());

        ss << slotTracker.getLocalSlot(value);
    }

    return ss.str();
}

std::string
getCurrentBacktrace()
{
    // http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
    StringStream ss;
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
                ss << "[bt]: (" << i << ") " << messages[i] << " : "
                   << real_name << "+" << offset_begin << offset_end
                   << "\n";
            }
            else
            {
                ss << "[bt]: (" << i << ") " << messages[i] << " : "
                   << mangled_name << "+" << offset_begin << offset_end
                   << "\n";
            }

            free(real_name);
        }
        else
            ss << "[bt]: (" << i << ") " << messages[i] << "\n";
    }

    free(messages);
    return ss.str();
}

} // namespace Canal
