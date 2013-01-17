#ifndef LIBCANAL_WIDENING_INTERFACE_H
#define LIBCANAL_WIDENING_INTERFACE_H

#include "Prereq.h"

namespace Canal {

class Domain;

namespace Widening {

class Interface
{
public:
    enum InterfaceKind {
        NumericalInfinityKind,
        PointersKind
    };

    const InterfaceKind mKind;

public:
    Interface(InterfaceKind kind) : mKind(kind)
    {
    }

    virtual ~Interface()
    {
    }

    InterfaceKind getKind() const
    {
        return mKind;
    }

    virtual void widen(const llvm::BasicBlock &wideningPoint,
                       Domain &first,
                       const Domain &second) = 0;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_INTERFACE_H
