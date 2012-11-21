#ifndef LIBCANAL_WIDENING_INTERFACE_H
#define LIBCANAL_WIDENING_INTERFACE_H

#include "Prereq.h"

namespace Canal {

class Domain;

namespace Widening {

class Interface
{
public:
    virtual ~Interface() {}

    virtual void widen(const llvm::BasicBlock &wideningPoint,
                       Domain &first,
                       const Domain &second) = 0;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_INTERFACE_H
