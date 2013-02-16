#include "ArrayStringTrie.h"
#include "Environment.h"
#include "Utils.h"
#include "IntegerUtils.h"

namespace Canal {
namespace Array {

TrieNode::TrieNode(const std::string &value)
    : mValue(value)
{ 
}

StringTrie::StringTrie(const Environment &environment,
                       const llvm::SequentialType &type)
    : Domain(environment, Domain::ArrayStringTrieKind),
      mIsBottom(true),
      mRoot(new TrieNode("")),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
}

StringTrie::StringTrie(const Environment &environment,
                       const llvm::SequentialType &type,
                       std::vector<Domain*>::const_iterator begin,
                       std::vector<Domain*>::const_iterator end)
    : Domain(environment, Domain::ArrayStringTrieKind),
      mIsBottom(true),
      mRoot(new TrieNode("")),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
    else
    {
        TrieNode *newNode = new TrieNode("");

        std::vector<Domain*>::const_iterator it = begin;
        for (; it != end; ++it)
        {
            if (!Integer::Utils::isConstant(**it))
                break;

            CANAL_ASSERT_MSG(8 == Integer::Utils::getBitWidth(**it),
                             "String requires 8-bit characters.");

            llvm::APInt constant;
            bool success = Integer::Utils::signedMin(**it, constant);
            CANAL_ASSERT(success);

            uint64_t c = constant.getZExtValue();
            if (c == 0 || c > 255)
                break;

            mIsBottom = false;
            newNode->mValue.append(1, char(c));
        }

        if (!mIsBottom)
        {
            mRoot->mChildren.push_back(newNode);
        }
    }
}

StringTrie::StringTrie(const Environment &environment,
                       const std::string &value)
    : Domain(environment, Domain::ArrayStringTrieKind),
      mIsBottom(false),
      mRoot(new TrieNode("")),
      mType(*llvm::ArrayType::get(llvm::Type::getInt8Ty(environment.getContext()),
                                  value.size()))
{
    TrieNode *newNode = new TrieNode(value);
    mRoot->mChildren.push_back(newNode);
}

StringTrie *
StringTrie::clone() const
{
    return new StringTrie(*this);
}

size_t
StringTrie::memoryUsage() const
{
    CANAL_NOT_IMPLEMENTED();
}

std::string
StringTrie::toString() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
StringTrie::setZero(const llvm::Value *place)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
StringTrie::operator==(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

bool StringTrie::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

StringTrie &
StringTrie::join(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

StringTrie &
StringTrie::meet(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
StringTrie::isBottom() const
{
    return mIsBottom;
}

void StringTrie::setBottom()
{
    mIsBottom = true;
    mRoot = NULL;
}

bool
StringTrie::isTop() const
{
    return !mIsBottom && (mRoot == NULL);
}

void StringTrie::setTop()
{
    mIsBottom = false;
    mRoot = NULL;
}

} // namespace Array
} // namespace Canal
