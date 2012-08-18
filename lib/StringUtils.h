#ifndef LIBCANAL_STRING_UTILS_H
#define LIBCANAL_STRING_UTILS_H

#include <inttypes.h>
#include <cstring>

namespace Canal {
namespace StringUtils {

/// Never returns NULL.
char *
xstrdup(const char *s);

/// Never returns NULL.
char *
xstrndup(const char *s, size_t n);

/// If the input contains character c in the current positon, move the
/// input pointer after the character, and return true. Otherwise do
/// not modify the input and return false.
bool
skipChar(const char **input, char c);

/// If the input contains one of allowed characters, move the input
/// pointer after that character, and return true.  Otherwise do not
/// modify the input and return false.
bool
skipCharLimited(const char **input, const char *allowed);

/// If the input contains one of allowed characters, store
/// the character to the result, move the input pointer after
/// that character, and return true. Otherwise do not modify
/// the input and return false.
bool
parseCharLimited(const char **input, const char *allowed, char *result);

/// If the input contains the character c one or more times, update it
/// so that the characters are skipped. Returns the number of characters
/// skipped, thus zero if **input does not contain c.
int
skipCharSequence(const char **input, char c);

/// If the input contains one or more characters from string chars,
/// move the input pointer after the sequence. Otherwise do not modify
/// the input.
/// @returns
/// The number of characters skipped.
int
skipCharSpan(const char **input, const char *chars);

/// If the input contains one or more characters from string accept,
/// create a string from this sequence and store it to the result, move
/// the input pointer after the sequence, and return the length of the
/// sequence.  Otherwise do not modify the input and return 0.
///
/// If this function returns nonzero value, the caller is responsible
/// to free the result.
int
parseCharSpan(const char **input, const char *accept, char **result);

/// If the input contains the string, move the input pointer after
/// the sequence. Otherwise do not modify the input.
/// @returns
/// Number of characters skipped. 0 if the input does not contain the
/// string.
int
skipString(const char **input, const char *string);

/// If the input contains digit 0-9, return it as a character
/// and move the input pointer after it. Otherwise return
/// '\0' and do not modify the input.
char
parseDigit(const char **input);

/// If the input contains [0-9]+, parse it, move the input pointer
/// after the number.
/// @returns
/// Number of parsed characters. 0 if input does not contain a number.
int
parseInt64(const char **input, int64_t *result);

/// If the input contains [0-9]+, parse it, move the input pointer
/// after the number.
/// @returns
/// Number of parsed characters. 0 if input does not contain a number.
int
parseUInt64(const char **input, uint64_t *result);

/// If the input contains 0x[0-9a-f]+, parse the number, and move the
/// input pointer after it.  Otherwise do not modify the input.
/// @returns
/// The number of characters read from input. 0 if the input does not
/// contain a hexadecimal number.
int
parseHexadecimalUInt64(const char **input, uint64_t *result);

} // namespace StringUtils
} // namespace Canal

#endif
