#ifndef CANAL_ABSTRACT_POINTER_H
#define CANAL_ABSTRACT_POINTER_H

#include "AbstractValue.h"
#include <set>

class AbstractPointer : public AbstractValue
{
public:
    std::set<AbstractValue*> mTargets;

    // Implementation of AbstractValue::clone().
    // Covariant return type -- it really overrides AbstractValue::clone().
    virtual AbstractPointer* clone() const;
    // Implementation of AbstractValue::merge().
    virtual void merge(const AbstractValue &v);
    // Implementation of AbstractValue::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of AbstractValue::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size);
    // Implementation of AbstractValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AbstractValue::printToStream().
    virtual void printToStream(llvm::raw_ostream &o) const;
};

#endif
