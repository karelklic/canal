#ifndef LIBCANAL_WIDENING_DATA_INTERFACE_H
#define LIBCANAL_WIDENING_DATA_INTERFACE_H

#include "Prereq.h"

namespace Canal {
namespace Widening {

class DataInterface
{
public:
    enum DataInterfaceKind {
        DataIterationCountKind
    };

    const DataInterfaceKind mKind;

public:
    DataInterface(DataInterfaceKind kind) : mKind(kind)
    {
    }

    virtual ~DataInterface()
    {
    }

    DataInterfaceKind getKind() const
    {
        return mKind;
    }

    virtual DataInterface *clone() const = 0;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_DATA_INTERFACE_H
