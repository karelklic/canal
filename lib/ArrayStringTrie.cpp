#include "ArrayStringTrie.h"
#include "Environment.h"
#include "Utils.h"
#include "IntegerUtils.h"
#include <iostream>

namespace Canal {
namespace Array {

bool
TrieNode::Compare::operator()(const TrieNode *first, const TrieNode *second) const
{
    return first->mValue < second->mValue;
}

TrieNode::TrieNode(const std::string &value)
    : mValue(value)
{ 
}

TrieNode::TrieNode(const TrieNode &node)
{
    mValue = node.mValue;

    std::set<TrieNode *, Compare>::const_iterator it = node.mChildren.begin(),
        itend = node.mChildren.end();

    for (; it != itend; ++it)
    {
        TrieNode *newNode = new TrieNode(**it);
        mChildren.insert(newNode);
    }
}

TrieNode::~TrieNode()
{
    std::set<TrieNode*, Compare>::const_iterator it = mChildren.begin(),
        itend = mChildren.end();

    for (; it != itend; ++it)
    {
        delete (*it);
    }
}

size_t
TrieNode::size()
{
    size_t size = mValue.size();
    std::set<TrieNode *, Compare>::const_iterator it = mChildren.begin(),
        itend = mChildren.end();

    for (; it != itend; ++it)
    {
        size += (*it)->size();
    }

    return size;
}

bool
TrieNode::operator==(const TrieNode &node) const
{
    if (this == &node)
        return true;

    if (mValue != node.mValue)
        return false;

    if (mChildren.size() != node.mChildren.size())
        return false;

    std::set<TrieNode*, Compare>::const_iterator first = mChildren.begin(),
        firstEnd = mChildren.end(),
        second = node.mChildren.begin(),
        secondEnd = node.mChildren.end();

    for (; first != firstEnd && second != secondEnd; ++first, ++second)
    {
        if ((**first == **second) == false)
            return false;
    }

    return true;
}

bool
TrieNode::operator!=(const TrieNode &node) const
{
    return !operator==(node); 
}

std::string
TrieNode::toString() const
{
    StringStream ss;
    ss << mValue;

    std::set<TrieNode *, Compare>::const_iterator it = mChildren.begin(),
        end = mChildren.end();

    if (mChildren.size() > 0)
        ss << "(" << (*it++)->toString();

    for (; it != end; ++it)
    {
        ss << "|" << (*it)->toString();
    }

    if (mChildren.size() > 0)
        ss << ")?";

    return ss.str();
}

size_t
TrieNode::getNumberOfMatchingSymbols(const std::string &value) const
{
    size_t result = 0;
    while (result < value.length() && result < mValue.length())
    {
        if (value[result] != mValue[result])
            break;
        result++;
    }
    return result;
}

TrieNode *
TrieNode::getMatchingChild(const std::string &value)
{
    std::set<TrieNode *, Compare>::const_iterator it = mChildren.begin(),
        end = mChildren.end();

    for (; it != end; ++it)
    {
        if ((*it)->getNumberOfMatchingSymbols(value) != 0)
            return *it;
    }

    return NULL;
}

void
TrieNode::split(const size_t index)
{
    std::string newValue = mValue.substr(index, mValue.length() - index);
    std::string oldValue = mValue.substr(0, index);
    TrieNode *newNode = new TrieNode(newValue);
    newNode->mIsActualString = true;
    //newNode->mChildren(mChildren);
    //mChildren.clear();
    mChildren.insert(newNode);
    mIsActualString = false;
    mValue = oldValue;
}

void
TrieNode::insert(const std::string &value)
{
    if (mChildren.size() == 0 || getMatchingChild(value) == NULL)
    {
        TrieNode *newNode = new TrieNode(value);
        newNode->mIsActualString = true;
        mChildren.insert(newNode);
    }
    else
    {
        TrieNode *matchingChild = getMatchingChild(value);
        size_t matchIndex =
            matchingChild->getNumberOfMatchingSymbols(value);

        if (matchingChild->mValue.length() != matchIndex)
            matchingChild->split(matchIndex);

        std::string newValue =
            value.substr(matchIndex, value.length() - matchIndex);
        
        if (newValue != "")
            matchingChild->insert(newValue);
    }
}

void
TrieNode::getRepresentedStrings(std::vector<std::string> &results,
                                const std::string &prefix)
{
    std::set<TrieNode *, Compare>::const_iterator it = mChildren.begin(),
        end = mChildren.end();

    for (; it != end; ++it)
    {
        std::string str = prefix + (*it)->mValue;
        (*it)->getRepresentedStrings(results, str);

        if ((*it)->mIsActualString)
            results.push_back(str);
    }
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
            newNode->mIsActualString = true;
            mRoot->mChildren.insert(newNode);
        }
        else
        {
            delete newNode;
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
    newNode->mIsActualString = true;
    mRoot->mChildren.insert(newNode);
}


StringTrie::~StringTrie()
{
    delete mRoot;
}


StringTrie *
StringTrie::clone() const
{
    return new StringTrie(*this);
}

size_t
StringTrie::memoryUsage() const
{
    size_t size = sizeof(StringTrie);
    size += mRoot->mValue.size();
    return size;
}

std::string
StringTrie::toString() const
{
    StringStream ss;
    ss << "stringTrie ";

    if (isTop())
        ss << "top";

    if (isBottom())
        ss << "bottom";

    ss << "\n";
        ss << "    type " << Canal::toString(mType) << "\n";

    if (!isBottom() && !isTop())
        ss << "    " << mRoot->toString() << "\n";

    return ss.str();
}

void
StringTrie::setZero(const llvm::Value *place)
{
    setTop();
}

bool
StringTrie::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const StringTrie &array = checkedCast<StringTrie>(value);
    if (isBottom() != array.isBottom())
        return false;

    if (!mIsBottom && (*mRoot != *array.mRoot))
        return false;

    return true;
}

bool StringTrie::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

StringTrie &
StringTrie::join(const Domain &value)
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

    const StringTrie &array = checkedCast<StringTrie>(value);

    if (isBottom())
    {
        delete mRoot;
        mRoot = new TrieNode(*array.mRoot);
    }
    else
    {
        std::vector<std::string> strings;
        array.mRoot->getRepresentedStrings(strings, array.mRoot->mValue);
        
        std::vector<std::string>::const_iterator it = strings.begin(),
            end = strings.end();

        for (; it != end; ++it)
        {
            mRoot->insert(*it);
        }
    }
   
    mIsBottom = false;
    return *this;
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
    delete mRoot;
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
    delete mRoot;
    mRoot = NULL;
}

} // namespace Array
} // namespace Canal
