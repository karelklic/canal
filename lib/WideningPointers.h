#ifndef LIBCANAL_WIDENING_POINTERS_H
#define LIBCANAL_WIDENING_POINTERS_H

#include "WideningInterface.h"

namespace Canal {
namespace Widening {

class Pointers : public Interface
{
public:
    Pointers() : Interface(Interface::PointersKind)
    {
    }

    virtual void widen(const llvm::BasicBlock &wideningPoint,
                       Domain &first,
                       const Domain &second);

    static bool classof(const Interface *value)
    {
        return value->getKind() == PointersKind;
    }
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_POINTERS_H
