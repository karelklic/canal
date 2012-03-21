#ifndef CANAL_ABSTRACT_INTEGER_H
#define CANAL_ABSTRACT_INTEGER_H

#include <set>

namespace AbstractInteger {

// Number of 1's in an integer type.
template <typename T>
int bitcount1(T x)
{
    int n = 0;
    if (x) {
        do {
            ++n;
        } while (x &= x-1);
        return n;
    }
}

// Abstracts integers as a bitfield.
template <typename T>
class Bits : public AbstractValue
{
public:
    // When a bit in Bits0 is 1, the value is known to contain zero at
    // this position.
    T mBits0;
    // When a bit in Bits1 is 1, the value is known to contain one at
    // this position.
    T mBits1;

public:
    // Initializes to the lowest value.
    Bits() : Bits0(0), Bits1(0) {}

    // Implementation of AbstractValue::clone().
    // Covariant return type -- it really overrides AbstractValue::clone().
    virtual Bits<T> *clone() const
    {
        return new Bits<T>(*this);
    }

    // Implementation of AbstractValue::merge().
    virtual void merge(const AbstractValue &V)
    {
        mBits0 |= ((const Bits<T>&)V).mBits0;
        mBits1 |= ((const Bits<T>&)V).mBits1;
    }

    // Implementation of AbstractValue::memoryUsage().
    virtual size_t memoryUsage() const
    {
        return 2 * sizeof(T);
    }

    // Implementation of AbstractValue::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size)
    {
        // Memory usage of this value cannot be lowered.
        return false;
    }

    // Implementation of AbstractValue::accuracy().
    virtual float accuracy() const
    {
        int sharedBits = bitcount1(mBits0 & mBits1);
        int allBits = sizeof(T) * 8;
        return 1.0 - (sharedBits / (float)allBits);
    }
};

// Abstracts integer values as a range min - max.
template <typename T>
class Range : public AbstractValue
{
public:
    bool mEmpty;

    bool mNegativeInfinity;
    T mFrom;

    bool mInfinity;
    T mTo;

public:
    // Initializes to the lowest value.
    Range() : mInfinity(false), mNegativeInfinity(false), mEmpty(true) {}

    // Covariant return type -- overrides AbstractValue::clone().
    virtual Range<T> *clone() const
    {
        return new Range<T>(*this);
    }
};

template <typename T>
class Enumeration : public AbstractValue
{
public:
    std::set<T> mValues;

public:
    // Covariant return type -- overrides AbstractValue::clone().
    virtual Enumeration<T> *clone() const
    {
        return new Enumeration<T>(*this);
    }
};


class AbstractChar : public AbstractValue
{
public:
    // Range is always used.
    struct Range
    {
        char mFrom;
        char mTo;
    };
    std::vector<Range> mRanges;

    // Possible values are only a supporting information, providing
    // better precision than ranges for some types of usage.
    bool mPossibleValuesValid;
    std::set<char> mPossibleValues;

    // Initializes to the lowest value.
    AbstractChar() : mPossibleValuesValid(true)
    {
    }

    virtual void merge(const AbstractValue& V)
    {
        const AbstractChar &Char = dynamic_cast<const AbstractChar&>(V);

        if (mPossibleValuesValid)
        {
            if (Char.mPossibleValuesValid)
                mPossibleValues.insert(Char.mPossibleValues.begin(), Char.mPossibleValues.end());
            else
            {
                mPossibleValuesValid = false;
                mPossibleValues.clear();
            }
        }
    }
};

} // namespace AbstractInteger

#endif
