#ifndef LIBCANAL_STRUCTURE_H
#define LIBCANAL_STRUCTURE_H

#include "AbstractValue.h"
#include <vector>

class AbstractStructure : public AbstractValue
{
 public:
  std::vector<AbstractValue> mMembers;
};

#endif // LIBCANAL_STRUCTURE_H
