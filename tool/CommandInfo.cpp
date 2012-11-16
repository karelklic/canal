#include "CommandInfo.h"
#include "State.h"
#include "Commands.h"
#include <cstdio>
#include <llvm/Module.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/PassManager.h>
#if LLVM_MAJOR > 2 || LLVM_MINOR > 8
#include <llvm/InitializePasses.h>
#endif // LLVM_MAJOR > 2 || LLVM_MINOR > 8
#include "lib/Operations.h"
#include "lib/Utils.h"
#include <sstream>

CommandInfo::CommandInfo(Commands &commands)
    : Command("info",
              "",
              "Generic command for showing things about the program being interpreted",
              "Generic command for showing things about the program being interpreted.",
              commands)
{
}

std::vector<std::string>
CommandInfo::getCompletionMatches(const std::vector<std::string> &args,
                                  int pointArg,
                                  int pointArgOffset) const
{
    std::vector<std::string> result;
    std::string arg = args[pointArg].substr(0, pointArgOffset);

    if (pointArg == 1)
    {
        if (0 == strncmp("module", arg.c_str(), arg.size()))
            result.push_back("module");

        if (0 == strncmp("functions", arg.c_str(), arg.size()))
            result.push_back("functions");

        if (0 == strncmp("function", arg.c_str(), arg.size()))
            result.push_back("function");
    }

    if (pointArg == 2 && args[1] == "function" && mCommands.getState())
    {
        const llvm::Module &module = mCommands.getState()->getModule();
        llvm::Module::const_iterator it = module.begin(),
            itend = module.end();

        for (; it != itend; ++it)
        {
            if (it->isDeclaration())
                continue;

            const char *name = it->getName().str().c_str();
            if (0 == strncmp(name, arg.c_str(), arg.size()))
                result.push_back(name);
        }
    }

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

#if LLVM_MAJOR > 2 || LLVM_MINOR > 8
    // Make sure CallGraph is added in the Pass Registry.  This is
    // necessary for LLVM newer than 2.8.
    llvm::PassRegistry &passRegistry =
        *llvm::PassRegistry::getPassRegistry();

    llvm::initializeBasicCallGraphPass(passRegistry);
#endif // LLVM_MAJOR > 2 || LLVM_MINOR > 8

    if (args[1] == "module")
        infoModule();
    else if (args[1] == "functions")
        infoFunctions();
    else if (args[1] == "function")
    {
        if (args.size() < 3)
        {
            printf("Missing function name.\n");
            return;
        }
        else if (args.size() > 3)
        {
            printf("Too many function names.\n");
            return;
        }

        infoFunction(args[2]);
    }
    else
    {
        printf("Undefined info command: \"%s\".  Try \"help info\".\n",
               args[1].c_str());
    }
}

void
CommandInfo::infoModule() const
{
    if (!mCommands.getState())
    {
        puts("No module is loaded.");
        return;
    }

    const llvm::Module &module = mCommands.getState()->getModule();
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
        llvm::Module::lib_iterator it = module.lib_begin();
        for (; it != module.lib_end(); ++it)
            printf("  %s\n", it->c_str());
    }
    else
        puts("Dependent Libraries: none");

    // Functions
    unsigned functionDeclarations = 0, functionDefinitions = 0;
    llvm::Module::const_iterator fit = module.begin(),
        fitend = module.end();

    for (; fit != fitend; ++fit)
    {
        if (fit->isDeclaration())
            ++functionDeclarations;
        else
            ++functionDefinitions;
    }

    printf("Function Declaration Count: %u\n", functionDeclarations);
    printf("Function Definition Count: %u\n", functionDefinitions);

    // Global variables
    unsigned globalVariables = 0;
    llvm::Module::const_global_iterator git = module.global_begin(),
        gitend = module.global_end();

    for (; git != gitend; ++git)
        ++globalVariables;

    printf("Global Variable Count: %u\n", globalVariables);

    // Global aliases
    unsigned globalAliases = 0;
    llvm::Module::const_alias_iterator ait = module.alias_begin(),
        aitend = module.alias_end();

    for (; ait != aitend; ++ait)
        ++globalAliases;

    printf("Global Alias Count: %u\n", globalAliases);
}

static size_t
getLoopCount(llvm::LoopInfoBase<llvm::BasicBlock,llvm::Loop>::iterator begin,
             llvm::LoopInfoBase<llvm::BasicBlock,llvm::Loop>::iterator end)
{
    size_t count = 0;
    llvm::LoopInfoBase<llvm::BasicBlock,llvm::Loop>::iterator it = begin;
    for (; it != end; ++it)
    {
        ++count;
        count += getLoopCount((*it)->begin(), (*it)->end());
    }

    return count;
}

struct FunctionEntry
{
    size_t mNaturalLoopCount;
    unsigned mCallsCount;
    unsigned mCalledCount;
};

struct FunctionInfo : public llvm::FunctionPass
{
    static char ID;
    std::map<llvm::Function*,struct FunctionEntry> mInfo;

    FunctionInfo() : llvm::FunctionPass(ID) {}

    virtual bool runOnFunction(llvm::Function &function)
    {
        struct FunctionEntry entry;

        llvm::LoopInfo &loopInfo = getAnalysis<llvm::LoopInfo>();
        entry.mNaturalLoopCount = getLoopCount(loopInfo.begin(),
                                               loopInfo.end());

        llvm::CallGraph &callGraph = getAnalysis<llvm::CallGraph>();
        llvm::CallGraphNode *node = callGraph[&function];
        CANAL_ASSERT(node);
        entry.mCallsCount = node->size();
        entry.mCalledCount = node->getNumReferences();

        mInfo[&function] = entry;
        return false;
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &analysisUsage) const
    {
        analysisUsage.setPreservesCFG();
        analysisUsage.addRequired<llvm::LoopInfo>();
        analysisUsage.addRequired<llvm::CallGraph>();
    }
};

char FunctionInfo::ID = 0;

static llvm::RegisterPass<FunctionInfo>
FunctionInfoRegistration("functioninfo", "FunctionInfo Pass",
                         true /* Only looks at CFG */,
                         false /* Analysis Pass */);

void
CommandInfo::infoFunctions() const
{
    if (!mCommands.getState())
    {
        puts("No module is loaded.");
        return;
    }

    llvm::Module &module = mCommands.getState()->getModule();

    std::vector<std::string> declarations, definitions;
    llvm::Module::const_iterator fit = module.begin(),
        fitend = module.end();

    for (; fit != fitend; ++fit)
    {
        if (fit->isDeclaration())
            declarations.push_back(fit->getName().str().c_str());
        else
            definitions.push_back(fit->getName().str().c_str());
    }

    if (declarations.empty())
        puts("Function Declarations: none");
    else
    {
        puts("Function Declarations:");
        std::sort(declarations.begin(), declarations.end());
        std::vector<std::string>::const_iterator it = declarations.begin();
        for (; it != declarations.end(); ++it)
            printf("  %s\n", it->c_str());
    }

    if (definitions.empty())
        puts("Function Definitions: none");
    else
    {
        puts("Function Definitions:");

        llvm::PassManager passManager;
        passManager.add(new llvm::LoopInfo());
        FunctionInfo *functionInfo = new FunctionInfo();
        passManager.add(functionInfo);
        passManager.run(module);

        std::sort(definitions.begin(), definitions.end());
        std::vector<std::string>::const_iterator it = definitions.begin();

        for (; it != definitions.end(); ++it)
        {
            printf("  %s\n", it->c_str());
            llvm::Function *fun = module.getFunction(it->c_str());
            if (!fun)
                continue;

            printf("    Argument Count: %zu\n", fun->arg_size());
            printf("    Basic Block Count: %zu\n",
                   fun->getBasicBlockList().size());

            printf("    Natural Loop Count: %zu\n",
                   functionInfo->mInfo[fun].mNaturalLoopCount);

            printf("    Function Calls Count: %u\n",
                   functionInfo->mInfo[fun].mCallsCount);

            printf("    Function Referenced Count: %u\n",
                   functionInfo->mInfo[fun].mCalledCount);
        }
    }
}

class LoopTree
{
public:
    std::vector<llvm::BasicBlock*> mLoop;
    std::vector<LoopTree> mSubLoops;

    LoopTree(llvm::Loop &loop)
    {
        mLoop = loop.getBlocks();
        llvm::Loop::iterator it = loop.begin();
        for (; it != loop.end(); ++it)
            mSubLoops.push_back(LoopTree(**it));
    }

    std::string toString(Canal::SlotTracker &slotTracker) const
    {
        std::stringstream ss;
        std::vector<llvm::BasicBlock*>::const_iterator
            it = mLoop.begin();

        ss << "loop: ";
        for (; it != mLoop.end(); ++it)
        {
            if (it != mLoop.begin())
                ss << "-";

            std::string name = Canal::getName(**it, slotTracker);
            if (0 == name.find("<label>:"))
                name = name.substr(strlen("<label>:"));

            ss << name;
        }

        ss << std::endl;

        std::vector<LoopTree>::const_iterator lit = mSubLoops.begin();
        for (; lit != mSubLoops.end(); ++lit)
        {
            std::string subLoop = lit->toString(slotTracker);
            ss << Canal::indent(subLoop, 8);
        }

        return ss.str();
    }
};

struct FunctionDetailedInfo : public llvm::FunctionPass
{
    static char ID;
    llvm::Function *mFunction;
    std::vector<LoopTree> mTopLevelLoops;

    FunctionDetailedInfo() : llvm::FunctionPass(ID) {}

    virtual bool runOnFunction(llvm::Function &function)
    {
        if (mFunction != &function)
            return false;

        llvm::LoopInfo &loopInfo = getAnalysis<llvm::LoopInfo>();
        llvm::LoopInfo::iterator it = loopInfo.begin();
        for (; it != loopInfo.end(); ++it)
            mTopLevelLoops.push_back(LoopTree(**it));


        llvm::CallGraph &callGraph = getAnalysis<llvm::CallGraph>();
        llvm::CallGraphNode *node = callGraph[&function];
        CANAL_ASSERT(node);

        return false;
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &analysisUsage) const
    {
        analysisUsage.setPreservesCFG();
        analysisUsage.addRequired<llvm::LoopInfo>();
        analysisUsage.addRequired<llvm::CallGraph>();
    }
};

char FunctionDetailedInfo::ID = 0;

static llvm::RegisterPass<FunctionDetailedInfo>
FunctionDetailedInfoRegistration(
    "functiondetailedinfo", "FunctionDetailedInfo Pass",
    true /* Only looks at CFG */,
    false /* Analysis Pass */);

void
CommandInfo::infoFunction(const std::string &name) const
{
    if (!mCommands.getState())
    {
        puts("No module is loaded.");
        return;
    }

    llvm::Module &module = mCommands.getState()->getModule();
    llvm::Function *func = module.getFunction(name.c_str());
    if (!func)
    {
        printf("Unknown function: \"%s\".\n", name.c_str());
        return;
    }

    Canal::SlotTracker &slotTracker =
        mCommands.getState()->getSlotTracker();

    llvm::Function::const_iterator it = func->begin(),
        itend = func->end();

    puts("Basic blocks:");
    for (; it != itend; ++it)
    {
        printf("  %s: %zu instructions\n",
               Canal::getName(*it, slotTracker).c_str(),
               it->size());
    }

    puts("Natural Loops:");
    llvm::PassManager passManager;
    passManager.add(new llvm::LoopInfo());
    FunctionDetailedInfo *functionInfo = new FunctionDetailedInfo();
    functionInfo->mFunction = func;
    passManager.add(functionInfo);
    passManager.run(module);

    std::vector<LoopTree>::const_iterator
        lit = functionInfo->mTopLevelLoops.begin();

    for (; lit != functionInfo->mTopLevelLoops.end(); ++lit)
        printf("%s", Canal::indent(lit->toString(slotTracker), 2).c_str());
 }
