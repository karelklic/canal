#ifndef CANAL_ABSTRACT_STRUCTURE_H
#define CANAL_ABSTRACT_STRUCTURE_H

#include "AbstractValue.h"
#include <vector>

class AbstractStructure : public AbstractValue
{
 public:
  std::vector<AbstractValue> Members;
};

#endif
