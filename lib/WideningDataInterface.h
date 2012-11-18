#ifndef LIBCANAL_WIDENING_DATA_INTERFACE_H
#define LIBCANAL_WIDENING_DATA_INTERFACE_H

#include "Prereq.h"

namespace Canal {
namespace Widening {

class DataInterface
{
public:
    virtual ~DataInterface() {}

    virtual DataInterface *clone() const = 0;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_DATA_INTERFACE_H
