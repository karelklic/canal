#ifndef LIBCANAL_MEMORY_VARIABLE_MAP_H
#define LIBCANAL_MEMORY_VARIABLE_MAP_H

#include "MemoryMap.h"
#include "Domain.h"

namespace Canal {
namespace Memory {

class VariableMap : public Map<Domain>
{
public:
    void joinGlobals(const VariableMap &map);

    void findNames(const std::string &prefix,
                   bool isGlobal,
                   SlotTracker &slotTracker,
                   std::vector<std::string> &result) const;
};


} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_VARIABLE_MAP_H
