#ifndef LIBCANAL_ARRAY_STRING_TRIE_H
#define LIBCANAL_ARRAY_STRING_TRIE_H

#include "Domain.h"

namespace Canal {
namespace Array {

class TrieNode
{
public:
    std::string mValue;
    std::vector<TrieNode*> mChildren;

public:
    TrieNode(const std::string &value);
};

class StringTrie : public Domain
{
public:
    TrieNode *mRoot;
    bool mIsBottom;
    const llvm::SequentialType &mType;

public:
    /// Standard constructor
    StringTrie(const Environment &environment,
               const llvm::SequentialType &type);

    StringTrie(const Environment &environment,
               const llvm::SequentialType &type,
               std::vector<Domain*>::const_iterator begin,
               std::vector<Domain*>::const_iterator end);

    StringTrie(const Environment &environment,
               const std::string &value);

    static bool classof(const Domain *value)
    {
        return value->getKind() == ArrayStringTrieKind;
    }

public: // Implementation of Domain

    virtual StringTrie *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual StringTrie &join(const Domain &value);

    virtual StringTrie &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_TRIE_H
