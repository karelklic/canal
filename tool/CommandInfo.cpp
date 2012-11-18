#include "CommandInfo.h"
#include "State.h"
#include "Commands.h"
#include "lib/Operations.h"
#include "lib/Utils.h"

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
    // Initialize passes
    llvm::PassRegistry &passRegistry =
        *llvm::PassRegistry::getPassRegistry();

    llvm::initializeCore(passRegistry);
    llvm::initializeAnalysis(passRegistry);
    llvm::initializeIPA(passRegistry);
#endif // LLVM_MAJOR > 2 || LLVM_MINOR > 8

    if (args[1] == "module")
        infoModule();
    else if (args[1] == "functions")
        infoFunctions();
    else if (args[1] == "function")
    {
        if (args.size() < 3)
        {
            llvm::outs() << "Missing function name.\n";
            return;
        }
        else if (args.size() > 3)
        {
            llvm::outs() << "Too many function names.\n";
            return;
        }

        infoFunction(args[2]);
    }
    else
    {
        llvm::outs() << "Undefined info command: \"" << args[1] << "\".  "
                     << "Try \"help info\".\n";
    }
}

void
CommandInfo::infoModule() const
{
    if (!mCommands.getState())
    {
        llvm::outs() << "No module is loaded.\n";
        return;
    }

    const llvm::Module &module = mCommands.getState()->getModule();
    llvm::outs() << "Identifier: " << module.getModuleIdentifier() << "\n"
                 << "Data layout: " << module.getDataLayout() << "\n"
                 << "Target: " << module.getTargetTriple() << "\n";

    // Endianess
    switch (module.getEndianness())
    {
    case llvm::Module::AnyEndianness:
        llvm::outs() << "Endianness: Any\n";
        break;
    case llvm::Module::LittleEndian:
        llvm::outs() << "Endianness: Little\n";
        break;
    case llvm::Module::BigEndian:
        llvm::outs() << "Endianness: Big\n";
        break;
    default:
        CANAL_DIE();
    }

    // Pointer size
    switch (module.getPointerSize())
    {
    case llvm::Module::AnyPointerSize:
        llvm::outs() << "Pointer size: Any\n";
        break;
    case llvm::Module::Pointer32:
        llvm::outs() << "Pointer size: 32 bit\n";
        break;
    case llvm::Module::Pointer64:
        llvm::outs() << "Pointer size: 64 bit\n";
        break;
    default:
        CANAL_DIE();
    }

    // Dependent libraries
    if (module.lib_size() > 0)
    {
        llvm::outs() << "Dependent Libraries:\n";
        llvm::Module::lib_iterator it = module.lib_begin();
        for (; it != module.lib_end(); ++it)
            llvm::outs() << "  " << *it << "\n";
    }
    else
        llvm::outs() << "Dependent Libraries: none\n";

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

    llvm::outs() << "Function Declaration Count: " << functionDeclarations << "\n"
                 << "Function Definition Count: " << functionDefinitions << "\n";

    // Global variables
    unsigned globalVariables = 0;
    llvm::Module::const_global_iterator git = module.global_begin(),
        gitend = module.global_end();

    for (; git != gitend; ++git)
        ++globalVariables;

    llvm::outs() << "Global Variable Count: " << globalVariables << "\n";

    // Global aliases
    unsigned globalAliases = 0;
    llvm::Module::const_alias_iterator ait = module.alias_begin(),
        aitend = module.alias_end();

    for (; ait != aitend; ++ait)
        ++globalAliases;

    llvm::outs() << "Global Alias Count: " << globalAliases << "\n";
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

    FunctionEntry() : mNaturalLoopCount(0), mCallsCount(0), mCalledCount(0)
    {
    }
};

class FunctionInfo : public llvm::FunctionPass
{
public:
    static char ID;
    std::map<llvm::Function*,struct FunctionEntry> mInfo;

    FunctionInfo() : llvm::FunctionPass(ID) {}

    virtual bool runOnFunction(llvm::Function &function)
    {
        if (function.isDeclaration())
            return false;

        struct FunctionEntry entry;

        llvm::LoopInfo &loopInfo = getAnalysis<llvm::LoopInfo>();
        entry.mNaturalLoopCount = getLoopCount(loopInfo.begin(),
                                               loopInfo.end());

        const llvm::CallGraph &callGraph = getAnalysis<llvm::CallGraph>();
        const llvm::CallGraphNode *node = callGraph[&function];
        CANAL_ASSERT(node);

        entry.mCallsCount = node->size();
        entry.mCalledCount = node->getNumReferences();

        mInfo[&function] = entry;
        return false;
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage &analysisUsage) const
    {
        analysisUsage.setPreservesAll();
        analysisUsage.addRequired<llvm::CallGraph>();
        analysisUsage.addRequired<llvm::LoopInfo>();
    }
};

char FunctionInfo::ID = 0;

static llvm::RegisterPass<FunctionInfo>
FunctionInfoRegistration("functioninfo",
                         "FunctionInfo Pass",
                         false /* Only looks at CFG */,
                         true /* Analysis Pass */);

void
CommandInfo::infoFunctions() const
{
    if (!mCommands.getState())
    {
        llvm::outs() << "No module is loaded.\n";
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
        llvm::outs() << "Function Declarations: none\n";
    else
    {
        llvm::outs() << "Function Declarations:\n";
        std::sort(declarations.begin(), declarations.end());
        std::vector<std::string>::const_iterator it = declarations.begin();
        for (; it != declarations.end(); ++it)
            llvm::outs() << "  " << *it << "\n";
    }

    if (definitions.empty())
        llvm::outs() << "Function Definitions: none\n";
    else
    {
        llvm::outs() << "Function Definitions:\n";

        llvm::PassManager passManager;
        FunctionInfo *functionInfo = new FunctionInfo();
        passManager.add(functionInfo);
        passManager.run(module);

        std::sort(definitions.begin(), definitions.end());
        std::vector<std::string>::const_iterator it = definitions.begin();

        for (; it != definitions.end(); ++it)
        {
            llvm::outs() << "  " << *it << "\n";
            llvm::Function *fun = module.getFunction(it->c_str());
            if (!fun)
                continue;

            llvm::outs() << "    Argument Count: "
                         << fun->arg_size() << "\n"
                         << "    Basic Block Count: "
                         << fun->getBasicBlockList().size() << "\n"
                         << "    Natural Loop Count: "
                         << functionInfo->mInfo[fun].mNaturalLoopCount << "\n"
                         << "    Function Calls Count: "
                         << functionInfo->mInfo[fun].mCallsCount << "\n"
                         << "    Function Referenced Count: "
                         << functionInfo->mInfo[fun].mCalledCount << "\n";
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
        llvm::Loop::iterator it = loop.begin(), itend = loop.end();
        for (; it != itend; ++it)
            mSubLoops.push_back(LoopTree(**it));
    }

    std::string toString(Canal::SlotTracker &slotTracker) const
    {
        Canal::StringStream ss;
        std::vector<llvm::BasicBlock*>::const_iterator
            it = mLoop.begin(), itend = mLoop.end();

        ss << "loop: ";
        for (; it != itend; ++it)
        {
            if (it != mLoop.begin())
                ss << "-";

            std::string name = Canal::getName(**it, slotTracker);
            if (0 == name.find("<label>:"))
                name = name.substr(strlen("<label>:"));

            ss << name;
        }

        ss << "\n";

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
        analysisUsage.setPreservesAll();
        analysisUsage.addRequired<llvm::LoopInfo>();
        analysisUsage.addRequired<llvm::CallGraph>();
    }
};

char FunctionDetailedInfo::ID = 0;

static llvm::RegisterPass<FunctionDetailedInfo>
FunctionDetailedInfoRegistration(
    "functiondetailedinfo", "FunctionDetailedInfo Pass",
    false /* Only looks at CFG */,
    false /* Analysis Pass */);

void
CommandInfo::infoFunction(const std::string &name) const
{
    if (!mCommands.getState())
    {
        llvm::outs() << "No module is loaded.\n";
        return;
    }

    llvm::Module &module = mCommands.getState()->getModule();
    llvm::Function *func = module.getFunction(name.c_str());
    if (!func)
    {
        llvm::outs() << "Unknown function: \"" << name << "\".\n";
        return;
    }

    Canal::SlotTracker &slotTracker =
        mCommands.getState()->getSlotTracker();

    llvm::Function::const_iterator it = func->begin(),
        itend = func->end();

    llvm::outs() << "Basic blocks:\n";
    for (; it != itend; ++it)
    {
        llvm::outs() << "  "
                     << Canal::getName(*it, slotTracker) << ": "
                     << it->size() << " instructions\n";
    }

    llvm::outs() << "Natural Loops:\n";
    llvm::PassManager passManager;
    FunctionDetailedInfo *functionInfo = new FunctionDetailedInfo();
    functionInfo->mFunction = func;
    passManager.add(functionInfo);
    passManager.run(module);

    std::vector<LoopTree>::const_iterator
        lit = functionInfo->mTopLevelLoops.begin();

    for (; lit != functionInfo->mTopLevelLoops.end(); ++lit)
        llvm::outs() << Canal::indent(lit->toString(slotTracker), 2);
}
