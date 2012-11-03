#ifndef LIBCANAL_WIDENING_POINTERS_H
#define LIBCANAL_WIDENING_POINTERS_H

#include "WideningInterface.h"

namespace Canal {
namespace Widening {

class Pointers : public Interface
{
public:
    virtual void widen(const llvm::BasicBlock &wideningPoint,
                       Domain &first,
                       const Domain &second);
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_POINTERS_H
