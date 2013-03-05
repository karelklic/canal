#ifndef FIELD_FIELDMINMAX_H
#define FIELD_FIELDMINMAX_H

#include "ProductMessageField.h"
#include "IntegerInterval.h"
#include <llvm/ADT/APInt.h>

namespace Canal {
namespace Field {

class MinMax : public Product::MessageField
{
public:
    Integer::Interval* mInterval;

    static bool classof(const MessageField* field)
    {
        return field->getKind() == FieldMinMaxKind;
    }
public:
    MinMax(const Integer::Interval& interval);
    MinMax(const MinMax& value);
    virtual ~MinMax();

    virtual MessageField* clone() const;
    virtual MessageField& meet(const MessageField& other);
};

} // namespace Field
} // namespace Canal

#endif // FIELD_FIELDMINMAX_H
