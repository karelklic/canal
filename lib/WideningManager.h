#ifndef LIBCANAL_WIDENING_MANAGER_H
#define LIBCANAL_WIDENING_MANAGER_H

#include "Prereq.h"
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
    void widen(const llvm::BasicBlock &wideningPoint,
               StateMap &first,
               const StateMap &second) const;

    void widen(const llvm::BasicBlock &wideningPoint,
               Domain &first,
               const Domain &second) const;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_MANAGER_H
