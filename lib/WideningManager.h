#ifndef LIBCANAL_WIDENING_MANAGER_H
#define LIBCANAL_WIDENING_MANAGER_H

namespace Canal {
namespace Widening {

class Manager
{
public:
    Manager();
    virtual ~Manager();

    void widen(const llvm::BasicBlock &wideningPoint,
               State &first,
               const State &second) const;

protected:
    void widen(const llvm::BasicBlock &wideningPoint,
               PlaceValueMap &first,
               const PlaceValueMap &second) const;

    void widen(const llvm::BasicBlock &wideningPoint,
               Domain &first,
               const Domain &second) const;

    std::vector<Interface*> mWidenings;
};

} // namespace Widening
} // namespace Canal

#endif // LIBCANAL_WIDENING_MANAGER_H
