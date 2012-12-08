#include "WrapperGcc.h"
#include "Utils.h"
#include <unistd.h>

WrapperGcc::WrapperGcc(int argCount, char **argVector)
    : mArgCount(argCount), mArgVector(argVector)
{
}

int
WrapperGcc::run()
{
    int result = runOriginalTool();
    runLlvm();
    return result;
}

int
WrapperGcc::runOriginalTool()
{
    std::string toolName = basename(mArgVector[0]);
    std::string toolPath = "/usr/bin/" + toolName;

    // First, run gcc.
    char **toolArgVector;
    toolArgVector = (char**)malloc((mArgCount + 1) * sizeof(char*));
    toolArgVector[0] = (char*)toolPath.c_str();
    for (int i = 1; i < mArgCount; ++i)
        toolArgVector[i] = mArgVector[i];

    toolArgVector[mArgCount] = NULL;

    pid_t pid = fork();
    if (pid < 0)
    {
        llvm::outs() << "Failed to fork: " << strerror(errno) << "\n";
        free(toolArgVector);
        return 127;
    }

    if (pid == 0)
    {
        execvp(toolArgVector[0], toolArgVector);
        _exit(127);
    }

    int status;
    safeWaitPid(pid, &status, 0);
    free(toolArgVector);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 127;
}

void
WrapperGcc::runLlvm()
{
    std::string errorInfo;
    llvm::raw_fd_ostream log("canal.log", errorInfo, llvm::raw_fd_ostream::F_Append);

    log << "======================================================\n";
    log << "time: " << llvm::sys::TimeValue::now().str() << "\n";
    log << "command: ";
    for (int i = 0; i < mArgCount; ++i)
    {
        log << mArgVector[i];
        if (i + 1 < mArgCount)
            log << " ";
    }

    log << "\n";

    std::string diagnosticString;
    llvm::raw_string_ostream diagnosticStream(diagnosticString);
    clang::TextDiagnosticPrinter *textDiagnosticPrinter
        = new clang::TextDiagnosticPrinter(diagnosticStream, clang::DiagnosticOptions());

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine diagnosticsEngine(diagID, textDiagnosticPrinter);

    clang::driver::Driver driver(mArgVector[0],
                                 llvm::sys::getHostTriple(),
                                 "a.out",
                                 /*IsProduction*/true,
                                 diagnosticsEngine);

    llvm::ArrayRef<const char*> args(mArgVector, mArgCount);
    clang::driver::InputArgList *inputArgList = driver.ParseArgStrings(args.slice(1));

    // TODO: do not suppress clang completely, just remove the option
    // from the list.  Sometimes these print options are used together
    // with other options doing real work.
    if (inputArgList->hasArg(clang::driver::options::OPT_dumpmachine) ||
        inputArgList->hasArg(clang::driver::options::OPT_dumpversion) ||
        inputArgList->hasArg(clang::driver::options::OPT__print_diagnostic_categories) ||
        inputArgList->hasArg(clang::driver::options::OPT__help) ||
        inputArgList->hasArg(clang::driver::options::OPT__help_hidden) ||
        inputArgList->hasArg(clang::driver::options::OPT__version) ||
        inputArgList->hasArg(clang::driver::options::OPT_v) ||
        inputArgList->hasArg(clang::driver::options::OPT__HASH_HASH_HASH) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_search_dirs) ||
        inputArgList->hasArg(clang::driver::options::OPT__version) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_file_name_EQ) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_prog_name_EQ) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_libgcc_file_name) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_multi_lib) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_multi_directory) ||
        inputArgList->hasArg(clang::driver::options::OPT_print_multi_os_directory))
    {
        log << "canal: supressing clang\n";
        return;
    }

    llvm::InitializeAllTargets();
    llvm::OwningPtr<clang::driver::Compilation>
        compilation(driver.BuildCompilation(args));

    int result = 0;
    const clang::driver::Command *failingCommand = 0;
    //if (compilation.get())
    //    result = driver.ExecuteCompilation(*compilation, failingCommand);

    // If result status is < 0, then the driver command signalled an
    // error.  In this case, generate additional diagnostic
    // information if possible.
    if (result < 0)
        driver.generateCompilationDiagnostics(*compilation, failingCommand);

    diagnosticStream.flush();
    if (!diagnosticString.empty())
        log << "diagnostics:\n" << diagnosticString;
}
