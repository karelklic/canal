#ifndef LIBCANAL_UTILS_H
#define LIBCANAL_UTILS_H

#include "Prereq.h"
#include <cstdlib>
#include <string>
#include <typeinfo>

/// Fatal error.  Writes a message to stderr and terminates the
/// application.
///
/// If you do not want to show any message, use CANAL_DIE instead.
#define CANAL_FATAL_ERROR(msg)                                   \
    {                                                            \
        llvm::errs() << __FILE__ << ":"                          \
                     << __LINE__ << "("                          \
                     << __FUNCTION__ << ") fatal error: "        \
                     << msg << "\n"                              \
                     << Canal::getCurrentBacktrace();            \
        abort();                                                 \
    }

/// Error.  Writes a message to stderr.  Program continues to run.
#define CANAL_ERROR(msg)                                         \
    {                                                            \
        llvm::errs() << __FILE__ << ":"                          \
                     << __LINE__ << "("                          \
                     << __FUNCTION__ << ") error: "              \
                     << msg << "\n"                              \
                     << Canal::getCurrentBacktrace();            \
    }

/// Assertion check.  On failure, the expression is written to stderr
/// and the application is terminated.
#define CANAL_ASSERT(expr)                                              \
    if (expr) ;                                                         \
    else                                                                \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << ") assert failed: "             \
                     << #expr << "\n"                                   \
                     << Canal::getCurrentBacktrace();                   \
        abort();                                                        \
    }

/// Assertion check.  On failuer, a message and the expression is
/// written to stderr and the application is terminated.
#define CANAL_ASSERT_MSG(expr, msg)                                     \
    if (expr) ;                                                         \
    else                                                                \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << ") assert failed: "             \
                     << msg << " [" << #expr << "]\n"                   \
                     << Canal::getCurrentBacktrace();                   \
        abort();                                                        \
    }

/// Termination.  The location where the program terminated is written
/// to stderr and the application is terminated.
#define CANAL_DIE()                                                     \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << "): dead code location reached\n" \
                     << Canal::getCurrentBacktrace();                   \
        exit(1);                                                        \
    }

/// Termination.  The location where the program terminated is written
/// to stderr and the application is terminated.
#define CANAL_DIE_MSG(msg)                                              \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << "): dead code location reached" \
                     << ": " << msg << "\n"                             \
                     << Canal::getCurrentBacktrace();                   \
        abort();                                                        \
    }

/// Report a function or a code block that is not implemented but it
/// should be.  The location is written to stderr. The application
/// continues to run.
#define CANAL_NOT_IMPLEMENTED()                                         \
    {                                                                   \
        llvm::errs() << __FILE__ << ":"                                 \
                     << __LINE__ << "("                                 \
                     << __FUNCTION__ << "): not implemented\n"          \
                     << Canal::getCurrentBacktrace();                   \
        abort();                                                        \
    }

namespace Canal {

class SlotTracker;

/// Get human readable string representation of llvm::APInt.
std::string toString(const llvm::APInt &num);

/// Get human readable string representation of llvm::Type.
std::string toString(const llvm::Type &type);

std::string indent(const std::string &input, int spaces);

std::string indentExceptFirstLine(const std::string &input, int spaces);

/// Get human readable name of a llvm::Value.
/// @param slotTracker
///   Slot Tracker with value's function assigned.
/// @returns
///   Empty string when no name was found.
std::string getName(const llvm::Value &value,
                    SlotTracker &slotTracker);

std::string getCurrentBacktrace();

template <class X, class Y> inline typename llvm::cast_retty<X, Y>::ret_type
llvmCast(const Y &val)
{
    CANAL_ASSERT_MSG(llvm::isa<X>(val),
                     "llvmCast<Ty>() argument of incompatible type!");

    return llvm::cast_convert_val<X, Y,
        typename llvm::simplify_type<Y>::SimpleType>::doit(val);
}


/// A raw_string_ostream that writes to an embedded std::string.  This
/// is a simple adaptor class.
class StringStream : public llvm::raw_string_ostream
{
    std::string mString;

public:
    StringStream() : llvm::raw_string_ostream(mString) {}
};

} // namespace Canal

#endif // LIBCANAL_UTILS_H
