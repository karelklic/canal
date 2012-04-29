#include "Commands.h"
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>

extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}

// List of all commands.
Commands gCommands;

static char *
complete_entry(const char *text, int state)
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
    // Allow conditional parsing of the ~/.inputrc file.
    rl_readline_name = (char*)"Canal";
    // Tell the completer that we want a crack first.
    rl_completion_entry_function = complete_entry;
    rl_terminal_name = getenv("TERM");
    rl_completer_word_break_characters = (char*)" \t\n";

    // Stifle the history list, remembering only the last 256 entries.
    stifle_history(256);

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
            add_history(expansion);
            gCommands.executeLine(expansion);
        }

        free(expansion);
        free(line);
    }

    return 0;
}
