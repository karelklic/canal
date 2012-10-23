#ifndef LIBCANAL_STATE_MAP_H
#define LIBCANAL_STATE_MAP_H

#include <map>

namespace llvm {
class Value;
} // namespace llvm

namespace Canal {

class Domain;

class StateMap
{
    /// llvm::Value represents a place in the program (an instruction,
    /// instance of llvm::Instruction).
    typedef std::map<const llvm::Value*, Domain*> Map;
    Map mMap;

public:
    StateMap() {}

    StateMap(const StateMap &map);

    ~StateMap();

    bool operator==(const StateMap &map) const;

    typedef Map::iterator iterator;
    typedef Map::const_iterator const_iterator;
    typedef Map::value_type value_type;
    typedef Map::size_type size_type;
    typedef Map::key_type key_type;

    iterator begin() { return mMap.begin(); }

    const_iterator begin() const { return mMap.begin(); }

    iterator end() { return mMap.end(); }

    const_iterator end() const { return mMap.end(); }

    size_type size() const { return mMap.size(); }

    iterator find(const key_type &x) { return mMap.find(x); }

    const_iterator find(const key_type &x) const { return mMap.find(x); }

    std::pair<iterator,bool> insert(const value_type &x) { return mMap.insert(x); }

    void merge(const StateMap &map);

    void insert(const llvm::Value &place, Domain *value);

};

} // namespace Canal

#endif // LIBCANAL_STATE_MAP_H
