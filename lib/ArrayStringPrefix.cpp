#include "ArrayStringPrefix.h"
#include "IntegerContainer.h"
#include "Utils.h"
#include "Environment.h"
#include "IntegerUtils.h"
#include "Constructors.h"

namespace Canal {
namespace Array {

StringPrefix::StringPrefix(const Environment &environment,
                           const llvm::SequentialType &type)
    : Domain(environment, Domain::ArrayStringPrefixKind),
      mIsBottom(true),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
}

StringPrefix::StringPrefix(const Environment &environment,
                           const llvm::SequentialType &type,
                           std::vector<Domain*>::const_iterator begin,
                           std::vector<Domain*>::const_iterator end)
    : Domain(environment, Domain::ArrayStringPrefixKind),
      mIsBottom(true),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
    else
    {
        std::vector<Domain*>::const_iterator it = begin;
        for (; it != end; ++it)
        {
            if (!Integer::Utils::isConstant(**it))
                break;

            CANAL_ASSERT_MSG(8 == Integer::Utils::getBitWidth(**it),
                             "String reqires 8-bit characters.");

            llvm::APInt constant;
            bool success = Integer::Utils::signedMin(**it, constant);
            CANAL_ASSERT(success);

            uint64_t c = constant.getZExtValue();
            if (c == 0 || c > 255)
                break;

            mIsBottom = false;
            mPrefix.append(1, (char)c);
        }
    }
}

StringPrefix::StringPrefix(const Environment &environment,
                           const std::string &value)
    : Domain(environment, Domain::ArrayStringPrefixKind),
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

    const StringPrefix &array = llvm::cast<StringPrefix>(value);
    if (isBottom() != array.isBottom())
        return false;

    if (!mIsBottom && (mPrefix != array.mPrefix))
        return false;

    return true;
}

bool
StringPrefix::operator<(const Domain &value) const
{
    if (this == &value)
        return false;

    const StringPrefix &array = llvm::cast<StringPrefix>(value);
    if (array.isBottom())
        return false;

    //if (!isBottom() && commonPrefix..._
    CANAL_NOT_IMPLEMENTED();

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

    const StringPrefix &array = llvm::cast<StringPrefix>(value);
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

    const StringPrefix &array = llvm::cast<StringPrefix>(value);
    if (isTop())
        mPrefix = array.mPrefix;
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

Domain *StringPrefix::extractelement(const Domain &index) const
{
    const llvm::Type &elementType = *mType.getElementType();
    Domain *result = mEnvironment.getConstructors().create(elementType);
    result->setTop();
    return result;
}

StringPrefix &
StringPrefix::insertelement(const Domain &array,
                            const Domain &element,
                            const Domain &index)
{
    setTop();
    return *this;
}

StringPrefix &
StringPrefix::shufflevector(const Domain &a,
                            const Domain &b,
                            const std::vector<uint32_t> &mask)
{
    setTop();
    return *this;
}

Domain *
StringPrefix::extractvalue(const std::vector<unsigned> &indices) const
{
    if (isTop())
    {
        // Cannot use const-correctness here because of differences
        // between LLVM versions.
        llvm::Type *type = (llvm::Type*)&mType;
        std::vector<unsigned>::const_iterator it = indices.begin(),
            itend = indices.end();

        for (; it != itend; ++it)
        {
            llvm::CompositeType *composite = llvm::cast<llvm::CompositeType>(type);
            type = composite->getTypeAtIndex(*it);
        }

        Domain *result = mEnvironment.getConstructors().create(*type);
        result->setTop();
        return result;
    }

    CANAL_ASSERT(indices.size() == 1);
    Domain *result = mEnvironment.getConstructors().create(*mType.getElementType());
    result->setTop();
    return result;
}

} // namespace Array
} // namespace Canal
