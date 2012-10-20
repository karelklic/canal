#ifndef LIBCANAL_WIDENING_DATA_INTERFACE_H
#define LIBCANAL_WIDENING_DATA_INTERFACE_H

namespace Canal {
namespace Widening {

class Interface
{
public:
    virtual void merge(const Data &data) = 0;
};


} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_DATA_INTERFACE_H
