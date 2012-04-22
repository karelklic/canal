#include <histedit.h>
#include <cstring>
#include <ctype.h>
#include <stdlib.h>

#include <readline/readline.h>
#include <readline/history.h>
//#include <editline/readline.h>

#include "Commands.h"

// When non-zero, this means the user is done using this program.
int done;

// List of all commands.
CommandList commandList;

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
    static std::vector<const char*> matches;
    if (state == 0)
        matches = commandList.getCompletionMatches(text);

    if (state < matches.size())
        return strdup(matches[state]);
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
    for (; !done;)
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
                commandList.executeLine(expansion);
            }
            free(expansion);
        }

        free(line);
    }

    return 0;
}
