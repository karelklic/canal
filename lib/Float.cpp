#include "Float.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Float {

Range::Range(const llvm::fltSemantics &semantics) : mFrom(semantics), mTo(semantics)
{
}

Range *
Range::clone() const
{
    return new Range(*this);
}

bool
Range::operator==(const Value& value) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Range::merge(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

size_t
Range::memoryUsage() const
{
    return sizeof(Range);
}

std::string
Range::toString(const State *state) const
{
    std::stringstream ss;
    ss << "Float::Range: {" << std::endl;
    ss << "    from: " << mFrom.convertToDouble() << std::endl;
    ss << "    to: " << mTo.convertToDouble() << std::endl;
    ss << "}";
    return ss.str();
}

} // namespace Float
} // namespace Canal

