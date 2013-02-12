#ifndef LIBCANAL_MEMORY_MAP_H
#define LIBCANAL_MEMORY_MAP_H

#include "SharedDataPointer.h"
#include "Utils.h"
#include <map>
#include <cstddef>
#if 0 // DEBUG
#include <iostream>
#endif

namespace Canal {
namespace Memory {

template<typename T>
class Map
{
    /// llvm::Value represents a place in the program (an instruction,
    /// instance of llvm::Instruction).
    typedef std::map<const llvm::Value*, SharedDataPointer<T> > InternalMap;

    InternalMap mMap;

public:
    typedef typename InternalMap::iterator iterator;
    typedef typename InternalMap::const_iterator const_iterator;
    typedef typename InternalMap::value_type value_type;
    typedef typename InternalMap::size_type size_type;
    typedef typename InternalMap::key_type key_type;

public:
    bool operator==(const Map<T> &map) const
    {
        if (size() != map.size())
            return false;

        const_iterator it1 = begin(), it1end = end();
        for (; it1 != it1end; ++it1)
        {
            const_iterator it2 = map.find(it1->first);
            if (it2 == map.end() || *it1->second != *it2->second)
            {
#if 0 // DEBUG
                // Print differences in state map for fixpoint calculation
                if (it2 == map.end())
                    std::cout << "Map ended" << std::endl;
                else
                {
                    std::cout << it1->second->toString()
                              << it2->second->toString()
                              << std::endl;
                }
#endif
                return false;
            }
        }

        return true;
    }

    iterator begin()
    {
        return mMap.begin();
    }

    const_iterator begin() const
    {
        return mMap.begin();
    }

    iterator end()
    {
        return mMap.end();
    }

    const_iterator end() const
    {
        return mMap.end();
    }

    size_type size() const
    {
        return mMap.size();
    }

    void clear()
    {
        mMap.clear();
    }

    iterator find(const key_type &x)
    {
        return mMap.find(x);
    }

    const_iterator find(const key_type &x) const
    {
        return mMap.find(x);
    }

    std::pair<iterator,bool> insert(const value_type &x)
    {
        return mMap.insert(x);
    }

    void join(const Map<T> &map)
    {
        const_iterator it2 = map.begin(), it2end = map.end();
        for (; it2 != it2end; ++it2)
        {
            iterator it1 = find(it2->first);
            if (it1 == end())
                insert(*it2);
            else if (*it1->second != *it2->second)
                it1->second.mutable_()->join(*it2->second);
        }
    }

    void insert(const llvm::Value &place, T *value)
    {
        CANAL_ASSERT_MSG(value, "NULL cannot be inserted to state.");

        iterator it = find(&place);
        if (it != end())
        {
            // Replace the old value.  SharedDataPointer will trigger a
            // delete if necessary.
            it->second = value;
        }
        else
            insert(value_type(&place, SharedDataPointer<T>(value)));
    }

    /// Get memory usage (used byte count) of this state map.
    size_t memoryUsage() const
    {
        size_t result = sizeof(Map<T>) + mMap.size() * sizeof(value_type);
        for (const_iterator it = begin(); it != end(); ++it)
            result += it->second->memoryUsage();

        return result;
    }
};

} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_VARIABLE_MAP_H
