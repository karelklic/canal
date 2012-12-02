#include "ArrayStringPrefix.h"
#include "IntegerContainer.h"
#include "Utils.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Array {

StringPrefix::StringPrefix(const Environment &environment,
                           const llvm::SequentialType &type)
    : Domain(environment), mIsBottom(true), mType(type)
{
    llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
}

StringPrefix::StringPrefix(const Environment &environment,
                           const llvm::SequentialType &type,
                           const std::vector<Domain*> &values)
    : Domain(environment), mIsBottom(true), mType(type)
{
    llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());

    std::vector<Domain*>::const_iterator it = values.begin(),
        itend = values.end();

    if (mType.getElementType() != int8)
        setTop();

    for (; it != itend; ++it)
    {
        if (!isTop())
        {
            // TODO: get the characters out of values and append them
            // to mPrefix instead of setting to top.
            setTop();
        }

        delete *it;
    }
}

StringPrefix::StringPrefix(const Environment &environment,
                           const llvm::SequentialType &type,
                           Domain *size)
    : Domain(environment), mIsBottom(true), mType(type)
{
    llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();

    delete size;
}

StringPrefix::StringPrefix(const Environment &environment,
                           const std::string &value)
    : Domain(environment),
      mPrefix(value),
      mIsBottom(false),
      mType(*llvm::ArrayType::get(llvm::Type::getInt8Ty(environment.getContext()),
                                  value.size()))
{
}

StringPrefix *
StringPrefix::clone() const
{
    return new StringPrefix(*this);
}

size_t
StringPrefix::memoryUsage() const
{
    size_t size = sizeof(StringPrefix);
    size += mPrefix.size();
    return size;
}

std::string
StringPrefix::toString() const
{
    StringStream ss;
    ss << "string prefix ";

    if (isTop())
        ss << "top";

    if (isBottom())
        ss << "bottom";

    ss << "\n";

    if (!isBottom() && !isTop())
        ss << "    " << mPrefix << "\n";

    return ss.str();
}

void
StringPrefix::setZero(const llvm::Value *place)
{
    setTop();
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

bool
StringPrefix::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
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

StringPrefix &
StringPrefix::join(const Domain &value)
{
    if (isTop())
        return *this;

    if (value.isBottom())
        return *this;

    if (value.isTop())
    {
        setTop();
        return *this;
    }

    const StringPrefix &array = dynCast<const StringPrefix&>(value);

    if (isBottom())
        mPrefix = array.mPrefix;
    else
        mPrefix = commonPrefix(mPrefix, array.mPrefix);

    mIsBottom = false;
    return *this;
}

StringPrefix &
StringPrefix::meet(const Domain &value)
{
    if (isBottom())
        return *this;

    if (value.isTop())
        return *this;

    if (value.isBottom())
    {
        setBottom();
        return *this;
    }

    const StringPrefix &array = dynCast<const StringPrefix&>(value);
    if (isTop())
    {
        mPrefix = array.mPrefix;
    }
    else if (0 == array.mPrefix.compare(0, mPrefix.length(), mPrefix))
    {
        // Array is a prefix of us
        mPrefix = array.mPrefix;
    }
    else if (0 != mPrefix.compare(0, array.mPrefix.length(), array.mPrefix))
    {
        // We are not a prefix of the array.
        setBottom();
    }

    return *this;
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

float
StringPrefix::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

StringPrefix &
StringPrefix::add(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::fadd(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::sub(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::fsub(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::mul(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::fmul(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::udiv(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::sdiv(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::fdiv(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::urem(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::srem(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::frem(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::shl(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::lshr(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::ashr(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::and_(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::or_(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::xor_(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::icmp(const Domain &a, const Domain &b,
                   llvm::CmpInst::Predicate predicate)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::fcmp(const Domain &a, const Domain &b,
                   llvm::CmpInst::Predicate predicate)
{
    setTop();
    return *this;
}

const llvm::SequentialType &
StringPrefix::getValueType() const
{
    return mType;
}

Domain *
StringPrefix::getValueCell(uint64_t offset) const
{
    Domain *cell = mEnvironment.getConstructors().createInteger(8);
    cell->setTop();
    return cell;
}

void
StringPrefix::mergeValueCell(uint64_t offset, const Domain &value)
{
    setTop();
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
