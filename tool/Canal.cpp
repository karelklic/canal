#include "Commands.h"
#include "WrapperGcc.h"
#include "lib/WideningDataIterationCount.h"
#include "lib/InterpreterOperationsCallback.h"
#include "lib/Utils.h"
#include <string>
#include <cstring>
#include <cctype>
#include <cstdlib>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

// List of all commands.
Commands gCommands;

static llvm::cl::list<std::string>
gEval("eval",
      llvm::cl::desc("Execute a single Machine command. May be used multiple times."),
      llvm::cl::value_desc("COMMAND"),
      llvm::cl::ValueRequired);

static llvm::cl::alias
gEvalAlias("e",
           llvm::cl::aliasopt(gEval),
           llvm::cl::ValueRequired,
           llvm::cl::ZeroOrMore);

static llvm::cl::opt<std::string>
gFileName(llvm::cl::Positional, llvm::cl::desc("<program file>"));

// Completion function for readline.
static char *
completeEntry(const char *text, int state)
{
    static std::vector<std::string> matches;
    if (state == 0)
    {
        std::string buf(rl_line_buffer);
        if ((unsigned)rl_point >= buf.size())
            buf += std::string(text);

        matches = gCommands.getCompletionMatches(buf, rl_point);
    }

    if (state < (int)matches.size())
        return strdup(matches[state].c_str());
    else
        return NULL;
}

// Version printer
void printVersion()
{
    llvm::outs() << "Canal " << VERSION << "\n";
}

// Program entry function.
int
main(int argc, char **argv)
{
    // Check if the tool is acting like a compiler or linker.
    std::string programName = basename(argv[0]);

    if (programName == "gcc" ||
        programName == "g++")
    {
        return wrapGcc(argc, argv);
    }
    else if (programName == "ld")
        return wrapLd(argc, argv);

    // Initialize the readline library.
    // Allow conditional parsing of the ~/.inputrc file.
    rl_readline_name = (char*)"Canal";
    // Tell the completer that we want a crack first.
    rl_completion_entry_function = completeEntry;
    rl_terminal_name = getenv("TERM");
    rl_completer_word_break_characters = (char*)" \t\n";
    // Stifle the history list, remembering only the last 256 entries.
    stifle_history(256);

    // Parse command arguments.
    llvm::cl::SetVersionPrinter(printVersion);
    llvm::cl::ParseCommandLineOptions(argc, argv);

    // Convert file names from command line into commands loading
    // those files and run the commands.
    if (!gFileName.empty())
    {
        Canal::StringStream ss;
        ss << "file " << gFileName;
        gCommands.executeLine(ss.str());
    }

    // Execute all commands from command line options.
    for (unsigned i = 0; i != gEval.size(); ++i)
        gCommands.executeLine(gEval[i]);

    // Loop reading and executing user input lines until the user
    // quits.
    while (true)
    {
        char *line = readline("(canal) ");
        if (!line)
            break;

        char *expansion;
        int result = history_expand(line, &expansion);
        if (result < 0 || result == 2)
            llvm::errs() << expansion << "\n";
        else
        {
            if (strlen(expansion) > 0)
                add_history(expansion);

            gCommands.executeLine(expansion);
        }

        free(expansion);
        free(line);
    }

    return 0;
}
