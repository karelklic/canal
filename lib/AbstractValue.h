#ifndef CANAL_ABSTRACT_VALUE_H
#define CANAL_ABSTRACT_VALUE_H

#include <cstddef>

class AbstractValue
{
public:
  // Create a copy of this value.
  virtual AbstractValue *clone() const = 0;

  // Merge another value into this one.
  virtual void merge(const AbstractValue &v) = 0;

  // Get memory usage (used byte count) of this value.
  virtual size_t memoryUsage() const = 0;

  // Decrease memory usage of this value below the provided size in
  // bytes.  Returns true if the memory usage was limited, false when
  // it was not possible.
  virtual bool limitMemoryUsage(size_t size) = 0;

  // Get accuracy of the abstract value (0 - 1). In finite-height
  // lattices, it is determined by the position of the value in the
  // lattice.
  //
  // Accuracy 0 means that the value represents all possible values.
  // Accuracy 1 means that the value represents the most precise and
  // exact value.
  virtual float accuracy() const = 0;

  virtual void add(const AbstractValue &a, const AbstractValue &b);
  virtual void sub(const AbstractValue &a, const AbstractValue &b);
  virtual void mul(const AbstractValue &a, const AbstractValue &b);
  // Signed division.
  virtual void div(const AbstractValue &a, const AbstractValue &b);
  // Signed remainder.
  virtual void rem(const AbstractValue &a, const AbstractValue &b);
  virtual void shl(const AbstractValue &a, const AbstractValue &b);
  virtual void shr(const AbstractValue &a, const AbstractValue &b);
  virtual void and_(const AbstractValue &a, const AbstractValue &b);
  virtual void or_(const AbstractValue &a, const AbstractValue &b);
  virtual void xor_(const AbstractValue &a, const AbstractValue &b);
};

#endif
