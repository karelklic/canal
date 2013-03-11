#include "FieldMinMax.h"
#include "Utils.h"

namespace Canal {
namespace Field {

MinMax::MinMax(const Integer::Interval& interval)
  : MessageField(MessageField::FieldMinMaxKind)
{
    mInterval = interval.clone();
}

MinMax::MinMax(const MinMax& value): MessageField(value)
{
    mInterval = value.mInterval->clone();
}


MinMax::~MinMax()
{
    delete mInterval;
}


Product::MessageField* MinMax::clone() const
{
    return new MinMax(*this);
}


Product::MessageField& MinMax::meet(const Product::MessageField& other)
{
    const MinMax &minMax = checkedCast<MinMax>(other);
    mInterval->meet(*minMax.mInterval);
}


} // namespace Field
} // namespace Canal
