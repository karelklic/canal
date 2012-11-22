#include "Commands.h"
#include <string>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "lib/WideningDataIterationCount.h"
#include "lib/InterpreterOperationsCallback.h"
#include "lib/Utils.h"

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <argp.h>
#include <sysexits.h>
}

// List of all commands.
Commands gCommands;

static struct argp_option gOptions[] = {
    {"eval", 'e', "COMMAND", 0, "Execute a single Canal command. May be used multiple times."},
    {"no-missing", 'n', 0, 0, "Do not print out names of missing functions."},
    { 0 }
};

class Arguments
{
public:
    std::vector<std::string> mEvalCommands;
    std::string mFileName;
};

static error_t
parseArgument(int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    class Arguments *arguments = (class Arguments*)state->input;

    switch (key)
    {
    case 'e':
        arguments->mEvalCommands.push_back(arg);
        break;
    case ARGP_KEY_ARG:
        if (!arguments->mFileName.empty())
        {
            argp_usage(state);
            exit(EX_USAGE);
        }
        arguments->mFileName = arg;
        break;
    case 'n':
        llvm::outs() << "Not printing missing functions.\n";
        Canal::Interpreter::printMissing = false;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

// Our argp parser.
static struct argp gArgumentParser = {
    gOptions,
    parseArgument,
    "[FILE]",
    "Canal -- Abstract Interpreter"
};

static char *
completeEntry(const char *text, int state)
{
    static std::vector<std::string> matches;
    if (state == 0)
    {
        std::string buf(rl_line_buffer);
        if (rl_point >= buf.size())
            buf += std::string(text);

        matches = gCommands.getCompletionMatches(buf, rl_point);
    }

    if (state < matches.size())
        return strdup(matches[state].c_str());
    else
        return NULL;
}

int
main(int argc, char **argv)
{
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
    Arguments arguments;
    argp_parse(&gArgumentParser, argc, argv, 0, 0, &arguments);

    if (!arguments.mFileName.empty())
    {
        Canal::StringStream ss;
        ss << "file " << arguments.mFileName;
        gCommands.executeLine(ss.str());
    }

    std::vector<std::string>::const_iterator it = arguments.mEvalCommands.begin();
    for (; it != arguments.mEvalCommands.end(); ++it)
        gCommands.executeLine(*it);

    // Loop reading and executing lines until the user quits.
    while (true)
    {
        char *line = readline("(canal) ");
        if (!line)
            break;

        char *expansion;
        int result = history_expand(line, &expansion);
        if (result < 0 || result == 2)
            fprintf(stderr, "%s\n", expansion);
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
