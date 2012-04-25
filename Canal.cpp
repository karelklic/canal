#include <histedit.h>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Commands.h"

// When non-zero, this means the user is done using this program.
int gDone;

// List of all commands.
Commands gCommands;

// Strip whitespace from the start and end of STRING.  Return a
// pointer into STRING.
static char *
stripwhite(char *string)
{
   register char *s, *t;
   for (s = string; isspace(*s); s++);

   if (*s == 0)
      return (s);

   t = s + strlen (s) - 1;
   while (t > s && isspace (*t))
      t--;
   *++t = '\0';

   return s;
}

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

    // Stifle the history list, remembering only the last 7 entries.
    stifle_history(256);

    // Loop reading and executing lines until the user quits.
    for (; !gDone;)
    {
        char *line = readline("(canal) ");
        if (!line)
            break;

        /* Remove leading and trailing whitespace from the line.
           Then, if there is anything left, add it to the history list
           and execute it. */
        char *s = stripwhite(line);

        if (*s)
        {
            char *expansion;
            int result = history_expand(s, &expansion);
            if (result < 0 || result == 2)
                fprintf(stderr, "%s\n", expansion);
            else
            {
                add_history(expansion);
                gCommands.executeLine(expansion);
            }

            free(expansion);
        }

        free(line);
    }

    return 0;
}
