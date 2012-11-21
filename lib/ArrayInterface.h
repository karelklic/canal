#ifndef LIBCANAL_ARRAY_INTERFACE_H
#define LIBCANAL_ARRAY_INTERFACE_H

#include "Prereq.h"
#include <vector>
#include <inttypes.h>

namespace Canal {

class Domain;

namespace Array {

class Interface
{
public:
    virtual ~Interface() {}

    /// Gets the value representing the array item or items pointed by
    /// the provided offset.  Caller is responsible for deleting the
    /// returned value.
    Domain *getValue(const Domain &offset) const;

    /// Gets the value representing the array item pointed by the
    /// provided offset.  Caller is responsible for deleting the
    /// returned value.
    /// @note
    ///  The uint64_t offset variant exists because of the extractvalue
    ///  instruction, which provides exact numeric offsets.
    Domain *getValue(uint64_t offset) const;

    /// Get the array items pointed by the provided offset.  Returns
    /// internal array items that are owned by the array.  Caller must
    /// not delete the items.
    virtual std::vector<Domain*> getItem(const Domain &offset) const = 0;

    /// Get the array item pointed by the provided offset.  Returns
    /// internal array item that is owned by the array.  Caller must
    /// not delete the item.
    /// @note
    ///  The uint64_t offset variant exists because of the extractvalue
    ///  instruction, which provides exact numeric offsets.
    /// @note
    ///  For future array domains it might be necessary to extend this
    ///  method to return a list of values.
    virtual Domain *getItem(uint64_t offset) const = 0;

    /// @param value
    ///  The method does not take the ownership of this memory.  It
    ///  copies the contents of the value instead.
    virtual void setItem(const Domain &offset, const Domain &value) = 0;

    /// @param value
    ///  The method does not take the ownership of this memory.  It
    ///  copies the contents of the value instead.
    /// @note
    ///  The uint64_t offset variant exists because of the insertvalue
    ///  instruction, which provides exact numeric offsets.
    virtual void setItem(uint64_t offset, const Domain &value) = 0;
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_H
