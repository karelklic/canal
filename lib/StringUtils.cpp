#include "StringUtils.h"
#include "Utils.h"
#include <cerrno>

namespace Canal {
namespace StringUtils {

char *
xstrdup(const char *s)
{
    char *result = ::strdup(s);
    CANAL_ASSERT_MSG(result, "out of memory");
    return result;
}

char *
xstrndup(const char *s, size_t n)
{
    char *result = ::strndup(s, n);
    CANAL_ASSERT_MSG(result, "out of memory");
    return result;
}


bool
skipChar(const char **input, char c)
{
    if (**input != c)
        return false;
    ++*input;
    return true;
}

bool
skipCharLimited(const char **input, const char *allowed)
{
    if (!strchr(allowed, **input))
        return false;
    ++*input;
    return true;
}

bool
parseCharLimited(const char **input, const char *allowed, char *result)
{
    if (**input == '\0')
        return false;
    if (!strchr(allowed, **input))
        return false;
    *result = **input;
    ++*input;
    return true;
}

int
skipCharSequence(const char **input, char c)
{
    // Skip all the occurences of c.
    int count = 0;
    while (skipChar(input, c))
        ++count;

    return count;
}

int
skipCharSpan(const char **input, const char *chars)
{
    size_t count = strspn(*input, chars);
    if (0 == count)
        return count;
    *input += count;
    return count;
}

int
parseCharSpan(const char **input, const char *accept, char **result)
{
    size_t count = strspn(*input, accept);
    if (0 == count)
        return 0;
    *result = xstrndup(*input, count);
    *input += count;
    return count;
}

int
skipString(const char **input, const char *string)
{
    const char *localInput = *input;
    const char *localString = string;
    while (*localString && *localInput && *localInput == *localString)
    {
        ++localInput;
        ++localString;
    }
    if (*localString != '\0')
        return 0;
    int count = localInput - *input;
    *input = localInput;
    return count;
}

char
parseDigit(const char **input)
{
    char digit = **input;
    if (digit < '0' || digit > '9')
        return '\0';
    ++*input;
    return digit;
}

int
parseInt64(const char **input, int64_t *result)
{
    const char *localInput = *input;
    char *numstr;
    int length = parseCharSpan(&localInput,
                               "+-0123456789",
                               &numstr);
    if (0 == length)
        return 0;

    char *endptr;
    errno = 0;
    unsigned long long r = strtoll(numstr, &endptr, 10);
    bool failure = (errno || numstr == endptr || *endptr != '\0');
    free(numstr);
    if (failure) /* number too big or some other error */
        return 0;
    *result = r;
    *input = localInput;
    return length;
}

int
parseUInt64(const char **input, uint64_t *result)
{
    const char *localInput = *input;
    char *numstr;
    int length = parseCharSpan(&localInput,
                               "0123456789",
                               &numstr);
    if (0 == length)
        return 0;

    char *endptr;
    errno = 0;
    unsigned long long r = strtoull(numstr, &endptr, 10);
    bool failure = (errno || numstr == endptr || *endptr != '\0');
    free(numstr);
    if (failure) // The number is too big or some other error.
        return 0;
    *result = r;
    *input = localInput;
    return length;
}

int
parseHexadecimalUInt64(const char **input, uint64_t *result)
{
    const char *localInput = *input;
    if (!skipChar(&localInput, '0'))
        return 0;
    if (!skipChar(&localInput, 'x'))
        return 0;
    int count = 2;
    char *numstr;
    count += parseCharSpan(&localInput,
                           "abcdef0123456789",
                           &numstr);

    if (2 == count) // parseCharSpan returned 0.
        return 0;
    char *endptr;
    errno = 0;
    unsigned long long r = strtoull(numstr, &endptr, 16);
    bool failure = (errno || numstr == endptr || *endptr != '\0');
    free(numstr);
    if (failure) // The number is too big or some other error.
        return 0;
    *result = r;
    *input = localInput;
    return count;
}

} // namespace StringUtils
} // namespace Canal
