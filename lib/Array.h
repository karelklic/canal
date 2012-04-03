#ifndef CANAL_ARRAY_H
#define CANAL_ARRAY_H

namespace Canal {
namespace Array {

// We treat all array members as a single value.  This means, that all
// the operations on the array are merged and used to move the single
// value up in its lattice.
class SingleItem : public Value
{
    // TODO
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
