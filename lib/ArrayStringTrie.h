#ifndef LIBCANAL_ARRAY_STRING_TRIE_H
#define LIBCANAL_ARRAY_STRING_TRIE_H

#include <set>
#include "Domain.h"

namespace Canal {
namespace Array {

class TrieNode
{
    struct Compare
    {
        bool operator()(const TrieNode *first, const TrieNode *second) const;
    };

public:
    std::string mValue;
    
    std::set<TrieNode *, Compare> mChildren;
    
    bool mIsActualString;

public:

    TrieNode(const std::string &value);

    TrieNode(const TrieNode &root);

    ~TrieNode();

    size_t size();

    bool operator==(const TrieNode &node) const;

    bool operator!=(const TrieNode &node) const;

    std::string toString() const;

    size_t getNumberOfMatchingSymbols(const std::string &value) const;

    TrieNode *getMatchingChild(const std::string &value);

    void split(const size_t index);

    void insert(const std::string &value);

    void getRepresentedStrings(std::vector<std::string> &results,
                               const std::string &value);
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

    ~StringTrie();

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

    virtual float accuracy() const;

    virtual StringTrie &add(const Domain &a, const Domain &b);

    virtual StringTrie &fadd(const Domain &a, const Domain &b);

    virtual StringTrie &sub(const Domain &a, const Domain &b);

    virtual StringTrie &fsub(const Domain &a, const Domain &b);

    virtual StringTrie &mul(const Domain &a, const Domain &b);

    virtual StringTrie &fmul(const Domain &a, const Domain &b);

    virtual StringTrie &udiv(const Domain &a, const Domain &b);

    virtual StringTrie &sdiv(const Domain &a, const Domain &b);

    virtual StringTrie &fdiv(const Domain &a, const Domain &b);

    virtual StringTrie &urem(const Domain &a, const Domain &b);

    virtual StringTrie &srem(const Domain &a, const Domain &b);

    virtual StringTrie &frem(const Domain &a, const Domain &b);

    virtual StringTrie &shl(const Domain &a, const Domain &b);

    virtual StringTrie &lshr(const Domain &a, const Domain &b);

    virtual StringTrie &ashr(const Domain &a, const Domain &b);

    virtual StringTrie &and_(const Domain &a, const Domain &b);

    virtual StringTrie &or_(const Domain &a, const Domain &b);

    virtual StringTrie &xor_(const Domain &a, const Domain &b);

    virtual StringTrie &icmp(const Domain &a,
                             const Domain &b,
                             llvm::CmpInst::Predicate predicate);

    virtual StringTrie &fcmp(const Domain &a,
                             const Domain &b,
                             llvm::CmpInst::Predicate predicate);

    virtual Domain *extractelement(const Domain &index) const;

    virtual StringTrie &insertelement(const Domain &array,
                                      const Domain &element,
                                      const Domain &index);

    virtual StringTrie &shufflevector(const Domain &a,
                                      const Domain &b,
                                      const std::vector<uint32_t> &mask);

    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    virtual StringTrie &insertvalue(const Domain &aggregate,
                                    const Domain &element,
                                    const std::vector<unsigned> &indices);

    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);

    virtual Domain *load(const llvm::Type &type,
                         const std::vector<Domain*> &offsets) const;

    virtual StringTrie &store(const Domain &value,
                              const std::vector<Domain*> &offsets,
                              bool overwrite);

    virtual const llvm::SequentialType &getValueType() const { return mType; }

};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_TRIE_H
