#include "WrapperGcc.h"
#include "Utils.h"
#include "lib/Utils.h"
#include <unistd.h>
#include <errno.h>

static int
run(int argc, char **argv)
{
    std::string toolName = basename(argv[0]);
    std::string toolPath = "/usr/bin/" + toolName;

    char **toolargv;
    toolargv = (char**)malloc((argc + 1) * sizeof(char*));
    toolargv[0] = (char*)toolPath.c_str();
    for (int i = 1; i < argc; ++i)
        toolargv[i] = argv[i];

    toolargv[argc] = NULL;

    pid_t pid = fork();
    if (pid < 0)
    {
        llvm::outs() << "Failed to fork: " << strerror(errno) << "\n";
        free(toolargv);
        return 127;
    }

    if (pid == 0)
    {
        execvp(toolargv[0], toolargv);
        _exit(127);
    }

    int status;
    safeWaitPid(pid, &status, 0);
    free(toolargv);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    else
        return 127;
}

static llvm::raw_ostream &
operator<<(llvm::raw_ostream &target,
           const clang::driver::ArgStringList &list)
{
    for (size_t i = 0; i < list.size(); ++i)
    {
        target << list[i];
        if (i + 1 < list.size())
            target << " ";
    }

    return target;
}

static llvm::raw_ostream &
operator<<(llvm::raw_ostream &target, char **argv)
{
    for (size_t i = 0; argv[i]; ++i)
    {
        target << argv[i];
        if (argv[i + 1])
            target << " ";
    }

    return target;
}

static llvm::raw_ostream &
operator<<(llvm::raw_ostream &target,
           const clang::driver::Command &command)
{
    target << " - command: " << command.getExecutable() << "\n"
           << "   action: " << command.getSource().getClassName() << "\n"
           << "   arguments: " << command.getArguments() << "\n";

    return target;
}

static llvm::raw_ostream &
operator<<(llvm::raw_ostream &target, const clang::driver::JobList &jobList)
{
    clang::driver::JobList::const_iterator it = jobList.begin();
    for (; it != jobList.end(); ++it)
    {
        if ((*it)->getKind() == clang::driver::Job::JobListClass)
        {
            const clang::driver::JobList &subList =
                Canal::llvmCast<clang::driver::JobList>(**it);

            target << subList;
        }
        else
        {
            const clang::driver::Command &command =
                Canal::llvmCast<clang::driver::Command>(**it);

            target << command;
        }
    }

    return target;
}

static clang::driver::ArgStringList
filterDriverArguments(int argc,
                      char **argv,
                      clang::driver::Driver &driver)
{
    clang::driver::ArgStringList result;
    result.push_back(argv[0]);

#if LLVM_VERSION_MAJOR > 2 || LLVM_VERSION_MINOR > 8
    clang::driver::InputArgList *inputArgList =
        driver.ParseArgStrings(argv + 1);
#else
    clang::driver::InputArgList *inputArgList =
        driver.ParseArgStrings(const_cast<const char**>(argv + 1),
                               const_cast<const char**>(argv + argc));
#endif

    if (inputArgList->hasArg(clang::driver::options::OPT__HASH_HASH_HASH))
        return result;

    clang::driver::InputArgList::const_iterator it = inputArgList->begin();
    for (; it != inputArgList->end(); ++it)
    {
        if ((*it)->getOption().matches(clang::driver::options::OPT_dumpmachine)
            || (*it)->getOption().matches(clang::driver::options::OPT_dumpversion)
            || (*it)->getOption().matches(clang::driver::options::OPT__print_diagnostic_categories)
            || (*it)->getOption().matches(clang::driver::options::OPT__help)
            || (*it)->getOption().matches(clang::driver::options::OPT__help_hidden)
            || (*it)->getOption().matches(clang::driver::options::OPT__version)
            || (*it)->getOption().matches(clang::driver::options::OPT_v)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_search_dirs)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_file_name_EQ)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_prog_name_EQ)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_libgcc_file_name)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_multi_lib)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_multi_directory)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_multi_directory)
            || (*it)->getOption().matches(clang::driver::options::OPT_print_multi_os_directory))
        {
            continue;
        }

        (*it)->render(*inputArgList, result);
    }

    delete inputArgList;
    return result;
}

static int
getCommandCount(const clang::driver::JobList &jobList)
{
    int result = 0;
    clang::driver::JobList::const_iterator it = jobList.begin();
    for (; it != jobList.end(); ++it)
    {
        if ((*it)->getKind() == clang::driver::Job::JobListClass)
        {
            const clang::driver::JobList &subList =
                Canal::llvmCast<clang::driver::JobList>(**it);

            result += getCommandCount(subList);
        }
        else
            ++result;
    }

    return result;
}

static int
getCommandCount(const clang::driver::JobList &jobList,
                clang::driver::Action::ActionClass actionClass)
{
    int result = 0;
    clang::driver::JobList::const_iterator it = jobList.begin();
    for (; it != jobList.end(); ++it)
    {
        if ((*it)->getKind() == clang::driver::Job::JobListClass)
        {
            const clang::driver::JobList &subList =
                Canal::llvmCast<clang::driver::JobList>(**it);

            result += getCommandCount(subList, actionClass);
        }
        else
        {
            const clang::driver::Command &command =
                Canal::llvmCast<clang::driver::Command>(**it);

            if (command.getSource().getKind() == actionClass)
                ++result;
        }
    }

    return result;
}

static clang::driver::Command *
assemblyOnly_changeCommandOutput(const clang::driver::Command &command,
                                 clang::driver::Driver &driver,
                                 llvm::raw_ostream &log)
{
    clang::driver::OptTable *table = clang::driver::createCC1OptTable();
    unsigned missingArgIndex, missingArgCount;
    clang::driver::InputArgList *inputArgList;
    inputArgList = table->ParseArgs(command.getArguments().begin(),
                                    command.getArguments().end(),
                                    missingArgIndex,
                                    missingArgCount);

    //inputArgList->eraseArg(clang::driver::cc1options::OPT_emit_obj);
    inputArgList->eraseArg(clang::driver::cc1options::OPT_coverage_file);
    inputArgList->eraseArg(clang::driver::cc1options::OPT_dependency_file);
    inputArgList->eraseArg(clang::driver::cc1options::OPT_MT);

    // Change the output.
    clang::driver::Arg *arg = inputArgList->getLastArg(clang::driver::cc1options::OPT_o);
    if (arg)
    {
        if (arg->getValues().size() == 1)
        {
            std::string output = arg->getValues()[0];
            arg->getValues().clear();
            arg->getValues().push_back(inputArgList->MakeArgString(output + ".llvm"));
        }
        else
            log << "error: -o does not have one value\n";
    }
    else
        log << "error: -o argument not found\n";

    clang::driver::ArgStringList arguments;
    clang::driver::InputArgList::const_iterator it = inputArgList->begin();
    for (; it != inputArgList->end(); ++it)
        (*it)->render(*inputArgList, arguments);

    clang::driver::Command *newCommand;
    newCommand = new clang::driver::Command(command.getSource(),
                                            command.getCreator(),
                                            "clang",
                                            arguments);

    delete table;
    return newCommand;
}

static void
assemblyOnly_modifyJobList(clang::driver::JobList &jobList,
                           clang::driver::Driver &driver,
                           llvm::raw_ostream &log)
{
    clang::driver::JobList::iterator it = jobList.begin();
    for (; it != jobList.end(); ++it)
    {
        if ((*it)->getKind() == clang::driver::Job::JobListClass)
        {
            clang::driver::JobList &subList =
                Canal::llvmCast<clang::driver::JobList>(**it);

            assemblyOnly_modifyJobList(subList, driver, log);
        }
        else
        {
            const clang::driver::Command *command =
                Canal::llvmCast<clang::driver::Command>(*it);

            *it = assemblyOnly_changeCommandOutput(*command, driver, log);
            delete command;
        }
    }
}

static bool
modifyJobList(clang::driver::JobList &jobList,
              clang::driver::Driver &driver,
              llvm::raw_ostream &log)
{
    int assembly = getCommandCount(
        jobList, clang::driver::Action::AssembleJobClass);

    int linker = getCommandCount(
        jobList, clang::driver::Action::LinkJobClass);

    int all = getCommandCount(jobList);

    if (all - assembly - linker != 0)
        return false;
    else if (assembly > 0 && linker == 0)
    {
        // Change the job to emit LLVM.
        // Change the output file.
        // Add command to merge the output file to the original output file.
        assemblyOnly_modifyJobList(jobList, driver, log);
        return true;
    }
    else if (assembly == 0 && linker == 1)
    {
        // Link the LLVM code inside the .o files as well, and include
        // it in the output file.
        // TODO
        return false;
    }
    else if (assembly > 0 && linker == 1)
    {
        // TODO
        return false;
    }
    else
        return false;

    return true;
}

static void
runClang(int argc, char **argv)
{
    std::string errorInfo;
    llvm::raw_fd_ostream log("canal.log",
                             errorInfo,
                             llvm::raw_fd_ostream::F_Append);

    log << "======================================================\n";
    log << "time: " << llvm::sys::TimeValue::now().str() << "\n";
    log << "command: " << argv << "\n";

    std::string diagnosticString;
    llvm::raw_string_ostream diagnosticStream(diagnosticString);
    clang::TextDiagnosticPrinter *textDiagnosticPrinter
        = new clang::TextDiagnosticPrinter(diagnosticStream,
                                           clang::DiagnosticOptions());

    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine diagnosticsEngine(diagID, textDiagnosticPrinter);

    clang::driver::Driver driver(argv[0],
                                 llvm::sys::getHostTriple(),
                                 "a.out",
                                 /*IsProduction*/true,
                                 diagnosticsEngine);

    clang::driver::ArgStringList clangArguments = filterDriverArguments(argc, argv, driver);
    //log << "filtered_command: " << clangArguments << "\n";

    // Terminate if there is nothing to do with clang.
    if (clangArguments.size() == 1)
        return;

    llvm::InitializeAllTargets();
    llvm::OwningPtr<clang::driver::Compilation>
        compilation(driver.BuildCompilation(clangArguments));

    clang::driver::JobList &jobList = compilation->getJobs();
    log << "original jobs (" << jobList.size() << "):\n" << jobList;

    bool success = modifyJobList(jobList, driver, log);
    if (success)
        log << "new jobs (" << jobList.size() << "):\n" << jobList;
    else
    {
        log << "error: Failed to modify job list.\n";
        log << "Skipping this command.\n";
        return;
    }

    // TODO: do not execute compilation for gcc tasks.
    int result = 0;
    const clang::driver::Command *failingCommand = 0;
    if (compilation.get())
        result = driver.ExecuteCompilation(*compilation, failingCommand);

    // If result status is < 0, then the driver command signalled an
    // error.  In this case, generate additional diagnostic
    // information if possible.
    if (result < 0)
        driver.generateCompilationDiagnostics(*compilation, failingCommand);

    diagnosticStream.flush();
    if (!diagnosticString.empty())
        log << "diagnostics:\n" << diagnosticString;
}

int
wrapGcc(int argc, char **argv)
{
    int result = run(argc, argv);
    runClang(argc, argv);
    return result;
}

int
wrapLd(int argc, char **argv)
{
    int result = run(argc, argv);
    return result;
}
