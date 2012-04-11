#ifndef CANAL_ARRAY_H
#define CANAL_ARRAY_H

namespace Canal {
namespace Array {

// We treat all array members as a single value.  This means, that all
// the operations on the array are merged and used to move the single
// value up in its lattice.
class SingleItem : public Value
{
public:
    Value *mItemValue;

    // Number of elements in the array.
    // It is either a Constant or Integer::Container.
    Value *mSize;

public:
    SingleItem();
    virtual ~SingleItem();

    // Implementation of Value::clone().
    // Covariant return type -- overrides Value::clone().
    virtual SingleItem *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;
};


// TODO: Delete.
class Range : public Value
{
public:
    enum {
        Ranges,
        ContextFreeGrammar
    } LimitType;

    // When LimitType is Ranges...
    struct SizeRange
    {
        int From;
        int To;
    };
    std::vector<SizeRange> SizeRanges;
    std::vector<AbstractVariable> Members;

    // When LimitType is ContextFreeGrammar...
    struct Symbol
    {
        // If Nonterminal is nonnegative, it is valid. Otherwise, Terminal
        // is valid.
        int Nonterminal;
        char Terminal;
    };

    struct ProductionRule
    {
        // Nonterminal.
        int From;
        std::vector<Symbol> To;
    };

    std::vector<ProductionRule> ProductionRules;
};

} // namespace Array
} // namespace Canal

#endif // CANAL_ARRAY_H
