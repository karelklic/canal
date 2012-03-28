#ifndef CANAL_ABSTRACT_POINTER_H
#define CANAL_ABSTRACT_POINTER_H

#include "AbstractValue.h"
#include <set>

class AbstractPointer : public AbstractValue
{
public:
    std::set<AbstractValue*> mTargets;

public:
    virtual ~AbstractPointer();

    // Implementation of AbstractValue::clone().
    // Covariant return type -- it really overrides AbstractValue::clone().
    virtual AbstractPointer* clone() const;
    // Implementation of AbstractValue::operator==().
    virtual bool operator==(const AbstractValue &value) const;
    // Implementation of AbstractValue::merge().
    virtual void merge(const AbstractValue &value);
    // Implementation of AbstractValue::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of AbstractValue::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size);
    // Implementation of AbstractValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AbstractValue::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;
};

#endif
