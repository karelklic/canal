#include "CommandInfo.h"
#include "State.h"
#include "Commands.h"
#include <cstdio>
#include <llvm/Module.h>
#include "../lib/Interpreter.h"
#include "../lib/Utils.h"

CommandInfo::CommandInfo(Commands &commands)
    : Command("info",
              "",
              "Generic command for showing things about the program being interpreted",
              "Generic command for showing things about the program being interpreted.",
              commands)
{
}

std::vector<std::string>
CommandInfo::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    std::vector<std::string> result;
    if (pointArg > 1)
        return result;

    std::string arg = args[pointArg].substr(0, pointArgOffset);

    if (0 == strncmp("module", arg.c_str(), arg.size()))
        result.push_back("module");

    return result;
}

void
CommandInfo::run(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        mCommands.executeLine("help info");
        return;
    }

    if (args[1] == "module")
        infoModule();
    else
        printf("Undefined info command: \"%s\".  Try \"help info\".\n", args[1].c_str());
}

void
CommandInfo::infoModule()
{
    if (!mCommands.mState || !mCommands.mState->mModule)
    {
        puts("No module is loaded.");
        return;
    }

    llvm::Module &module = *mCommands.mState->mModule;
    printf("Identifier: %s\n", module.getModuleIdentifier().c_str());
    printf("Data layout: %s\n", module.getDataLayout().c_str());
    printf("Target: %s\n", module.getTargetTriple().c_str());

    // Endianess
    switch (module.getEndianness())
    {
    case llvm::Module::AnyEndianness:
        puts("Endianness: Any");
        break;
    case llvm::Module::LittleEndian:
        puts("Endianness: Little");
        break;
    case llvm::Module::BigEndian:
        puts("Endianness: Big");
        break;
    default:
        CANAL_DIE();
    }

    // Pointer size
    switch (module.getPointerSize())
    {
    case llvm::Module::AnyPointerSize:
        puts("Pointer size: Any");
        break;
    case llvm::Module::Pointer32:
        puts("Pointer size: 32 bit");
        break;
    case llvm::Module::Pointer64:
        puts("Pointer size: 64 bit");
        break;
    default:
        CANAL_DIE();
    }

    // Dependent libraries
    if (module.lib_size() > 0)
    {
        puts("Dependent Libraries:");
        for (llvm::Module::lib_iterator it = module.lib_begin(); it != module.lib_end(); ++it)
            printf("  %s\n", it->c_str());
    }
    else
        puts("Dependent Libraries: none");

    // Global Aliases
    // Named Metadata
    // Functions
}
