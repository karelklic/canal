#ifndef LIBCANAL_WIDENING_MANAGER_H
#define LIBCANAL_WIDENING_MANAGER_H

#include "Prereq.h"
#include "MemoryMap.h"
#include <vector>

namespace Canal {
namespace Widening {

class Manager
{
    std::vector<Interface*> mWidenings;

public:
    Manager();
    virtual ~Manager();

    void widen(const llvm::BasicBlock &wideningPoint,
               Memory::State &first,
               const Memory::State &second) const;

protected:
    template <typename T>
    void widen(const llvm::BasicBlock &wideningPoint,
               Memory::Map<T> &first,
               const Memory::Map<T> &second) const;

    void widen(const llvm::BasicBlock &wideningPoint,
               Domain &first,
               const Domain &second) const;

    void widen(const llvm::BasicBlock &wideningPoint,
               Memory::Block &first,
               const Memory::Block &second) const;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_MANAGER_H
