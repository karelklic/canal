#include "Float.h"
#include "Constant.h"
#include "Utils.h"

namespace Canal {
namespace Float {

Range::Range(const llvm::fltSemantics &semantics) : mFrom(semantics), mTo(semantics)
{
}

Range *Range::clone() const
{
    return new Range(*this);
}

bool Range::operator==(const Value& value) const
{
    CANAL_NOT_IMPLEMENTED();
}

void Range::merge(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

size_t Range::memoryUsage() const
{
    return sizeof(Range);
}

void Range::printToStream(llvm::raw_ostream &ostream) const
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Float
} // namespace Canal

