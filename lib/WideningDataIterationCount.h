#ifndef LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H
#define LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H

namespace Canal {
namespace Widening {
namespace Data {

class IterationCount : public Interface
{
public:
    IterationCount() : mCount(0) {}

    virtual void merge(const Data &data);

...
};

} // namespace Data
} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_DATA_ITERATION_COUNT_H
