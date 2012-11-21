#include "ArrayStringPrefix.h"
#include "IntegerContainer.h"
#include "Utils.h"

namespace Canal {
namespace Array {

StringPrefix::StringPrefix(const Environment &environment)
    : Domain(environment)
{
    mIsBottom = true;
}

StringPrefix::StringPrefix(const Environment &environment,
                     const std::string &value)
    : Domain(environment)
{
    mPrefix = value;
    mIsBottom = false;
}

StringPrefix *
StringPrefix::clone() const
{
    return new StringPrefix(*this);
}

StringPrefix *
StringPrefix::cloneCleaned() const
{
    return new StringPrefix(mEnvironment);
}

bool
StringPrefix::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const StringPrefix *array = dynCast<const StringPrefix*>(&value);
    if (!array)
        return false;

    if (mIsBottom != array->mIsBottom)
        return false;

    if (!mIsBottom && (mPrefix != array->mPrefix))
        return false;

    return true;
}

static std::string
commonPrefix(std::string first, std::string second)
{
    if (first.length() > second.length())
        first.swap(second);

    size_t i;
    for (i = 0; i < first.length(); i++)
    {
        if (first[i] != second[i])
            break;
    }

    return first.substr(0, i);
}

void
StringPrefix::merge(const Domain &value)
{
    const StringPrefix &array = dynCast<const StringPrefix&>(value);

    if (mIsBottom && !array.mIsBottom)
    {
        mIsBottom = false;
        mPrefix = array.mPrefix;
    }

    if (!mIsBottom && !array.mIsBottom)
    {
        mPrefix = commonPrefix(mPrefix, array.mPrefix);
    }
}


size_t
StringPrefix::memoryUsage() const
{
    size_t size = sizeof(StringPrefix);
    size += mPrefix.size() * sizeof(Domain*);
    return size;
}

std::string
StringPrefix::toString() const
{
    StringStream ss;
    ss << "arrayStringPrefix\n";
    ss << mPrefix;
    return ss.str();
}

void
StringPrefix::setZero(const llvm::Value *place)
{
    setTop();
}

void
StringPrefix::add(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::fadd(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::sub(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::fsub(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::mul(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::fmul(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::udiv(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::sdiv(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::fdiv(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::urem(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::srem(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::frem(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::shl(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::lshr(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::ashr(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::and_(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::or_(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::xor_(const Domain &a, const Domain &b)
{
    setTop();
}

void
StringPrefix::icmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    setTop();
}

void
StringPrefix::fcmp(const Domain &a, const Domain &b,
                llvm::CmpInst::Predicate predicate)
{
    setTop();
}

bool
StringPrefix::isBottom() const
{
    return mIsBottom;
}

void
StringPrefix::setBottom()
{
    mIsBottom = true;
    mPrefix = "";
}

bool
StringPrefix::isTop() const
{
    return !mIsBottom && (mPrefix == "");
}

void
StringPrefix::setTop()
{
    mIsBottom = false;
    mPrefix = "";
}

std::vector<Domain*>
StringPrefix::getItem(const Domain &offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

Domain *
StringPrefix::getItem(uint64_t offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
StringPrefix::setItem(const Domain &offset, const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
StringPrefix::setItem(uint64_t offset, const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Array
} // namespace Canal
