#ifndef LIBCANAL_UTILS_H
#define LIBCANAL_UTILS_H

#include "Prereq.h"
#include <cstdlib>
#include <string>
#include <typeinfo>
#include "SuperPtr.h"

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
/// Detect bare type - type without constness, reference nor pointer
template <typename X>
struct bare_type {
    typedef X type;
    static const bool constant = false;
    static const bool reference = false;
    static const bool pointer = false;
};

template <typename X>
struct bare_type<X&> {
    typedef X type;
    static const bool constant = false;
    static const bool reference = true;
    static const bool pointer = false;
};

template <typename X>
struct bare_type<X*> {
    typedef X type;
    static const bool constant = false;
    static const bool reference = false;
    static const bool pointer = true;
};

template <typename X>
struct bare_type<const X&> {
    typedef X type;
    static const bool constant = true;
    static const bool reference = true;
    static const bool pointer = false;
};

template <typename X>
struct bare_type<const X> {
    typedef X type;
    static const bool constant = true;
    static const bool reference = false;
    static const bool pointer = false;
};

template <typename X>
struct bare_type<const X*> {
    typedef X type;
    static const bool constant = true;
    static const bool reference = false;
    static const bool pointer = true;
};

/// Try dynamic cast to SuperPtr<X>&
template<typename X, typename Y>
struct SuperPtrCast {
    /// Try to do dynamic cast and return modifiable reference to element
    static inline X
    modifiable(Y &val) {
        typedef typename bare_type<X>::type type;
        SuperPtr<type>& type_check = dynamic_cast<SuperPtr<type>& >(val);
        return type_check.modifiable();
    }
    /// Try to do dynamic cast and return const reference to element
    static inline X
    constant(const Y &val) {
        typedef typename bare_type<X>::type type;
        const SuperPtr<type>& type_check = dynamic_cast<const SuperPtr<type>& >(val);
        return (X) type_check;
    }
};
/// Try dynamic cast to SuperPtr<X>*
template<typename X, typename Y>
struct SuperPtrCast<X*, Y> {
    /// Try to do dynamic cast and return modifiable pointer to element
    static inline X*
    modifiable(Y* val) {
        typedef typename bare_type<X>::type type;
        SuperPtr<type>* type_check = dynamic_cast<SuperPtr<type>* >(val);
        return (type_check == NULL ? NULL : &type_check->modifiable());
    }
    /// Try to do dynamic cast and return const pointer to element
    static inline X*
    constant(const Y* val) {
        typedef typename bare_type<X>::type type;
        const SuperPtr<type>* type_check = dynamic_cast<const SuperPtr<type>* >(val);
        return (type_check == NULL ? NULL : (X*) *type_check);;
    }
};
/// Static if - if X is not const, call modifable (which takes X)
template<typename X, typename Y, typename = void>
struct static_if_cast {
    /// Takes reference
    static inline X static_if(Y& val) {
        return SuperPtrCast<X, Y>::modifiable(val);
    }
    /// Takes pointer
    static inline X static_if(Y* val) {
        return SuperPtrCast<X, Y>::modifiable(val);
    }
};
/// Static if - if X is ot const, call constant (which takes const X)
template<typename X, typename Y>
struct static_if_cast<X, Y, typename enable_if<bare_type<X>::constant>::type > {
    /// Takes reference
    static inline X static_if(Y& val) {
        return SuperPtrCast<X, Y>::constant(val);
    }
    /// Takes pointer
    static inline X static_if(Y* val) {
        return SuperPtrCast<X, Y>::constant(val);
    }
};
/// dynCastStruct - auxiliary class for dynCast; X is NOT subclass of Domain -> no need to try SuperPtr<X>
template<typename X, typename Y, typename = void>
struct dynCastStruct {
    /// Cast for reference
    static inline X cast(Y &val)
    {
        try
        {
            return dynamic_cast<X>(val);
        }
        catch (std::bad_cast exception) {
            CANAL_FATAL_ERROR(exception.what());
        }
    }
    /// Cast for constant reference
    static inline X cast_const(const Y &val)
    {
        try
        {
            return dynamic_cast<X>(val);
        }
        catch (std::bad_cast exception)
        {
            CANAL_FATAL_ERROR(exception.what()
                              << " from " << typeid(val).name()
                              << " to " << typeid(X).name());
        }
    }
    /// Cast for pointer
    static inline X cast(Y *val)
    {
        try
        {
            return dynamic_cast<X>(val);
        }
        catch (std::bad_cast exception) {
            CANAL_FATAL_ERROR(exception.what());
        }
    }
    /// Cast for constant pointer
    static inline X cast_const(const Y *val)
    {
        try
        {
            return dynamic_cast<X>(val);
        }
        catch (std::bad_cast exception)
        {
            CANAL_FATAL_ERROR(exception.what()
                              << " from " << typeid(val).name()
                              << " to " << typeid(X).name());
        }
    }
};
/// dynCastStruct - auxiliary class for dynCast; X is subclass of Domain -> try SuperPtr<X> if cast for X fails
template<typename X, typename Y>
struct dynCastStruct<X, Y,
        typename enable_if<is_base_of<Domain, typename bare_type<X>::type>::value>::type
       > {
    /// Cast for reference -> try SuperPtr<X> if X is not const, const SuperPtr<X> if X is const
    static inline X cast(Y &val)
    {
        try
        {
            return dynamic_cast<X>(val);
        }
        catch (std::bad_cast exception) {
            try { //Try SuperPtr - if fails, do nothing
                return static_if_cast<X, Y>::static_if(val);
            }
            catch (...) {}
            CANAL_FATAL_ERROR(exception.what());
        }
    }
    /// Cast for const reference -> try const SuperPtr<X>
    static inline X cast_const(const Y &val)
    {
        try
        {
            return dynamic_cast<X>(val);
        }
        catch (std::bad_cast exception)
        {
            try { //Try SuperPtr - if fails, do nothing
                return SuperPtrCast<X, Y>::constant(val);
            }
            catch (...) {}
            CANAL_FATAL_ERROR(exception.what()
                              << " from " << typeid(val).name()
                              << " to " << typeid(X).name());
        }
    }
    /// Cast for pointer -> try SuperPtr<X>* if X is not const, const SuperPtr<X>* if X is const
    static inline X cast(Y *val)
    {
        X x = dynamic_cast<X>(val);
        if (x == NULL) { //Try SuperPtr - if fails, do nothing -> returns NULL either way
            return static_if_cast<X, Y>::static_if(val);
        }
        return x;
    }
    /// Cast for const pointer -> try const SuperPtr<X>*
    static inline X cast_const(const Y *val)
    {
        X x = dynamic_cast<X>(val);
        if (x == NULL) { //Try SuperPtr - if fails, do nothing -> returns NULL either way
            return SuperPtrCast<X, Y>::constant(val);
        }
        return x;
    }
};
/// dynCast for reference
template <typename X, typename Y> inline X
dynCast(Y &val)
{
    return dynCastStruct<X, Y>::cast(val);
}
/// dynCast for const reference
template <typename X, typename Y> inline X
dynCast(const Y &val)
{
    return dynCastStruct<X, Y>::cast_const(val);
}
/// dynCast for pointer
template <typename X, typename Y> inline X
dynCast(Y *val)
{
    return dynCastStruct<X, Y>::cast(val);
}
/// dynCast for const pointer
template <typename X, typename Y> inline X
dynCast(const Y *val)
{
    return dynCastStruct<X, Y>::cast_const(val);
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
