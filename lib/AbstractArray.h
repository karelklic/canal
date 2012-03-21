#ifndef CANAL_ABSTRACT_ARRAY_H
#define CANAL_ABSTRACT_ARRAY_H

namespace AbstractArray {

class Range : public AbstractValue
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

}

#endif
