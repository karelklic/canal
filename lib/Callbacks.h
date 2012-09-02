#ifndef LIBCANAL_CALLBACKS_H
#define LIBCANAL_CALLBACKS_H

namespace Canal {

/// Universal class for program behavior monitoring.  It can be used
/// via inheritance to detect invalid operations, to find static
/// analysis precision bottlenecks, to identify situations that reach
/// boundaries of implemented techniques, and to drive custom static
/// analysis techniques.
class Callbacks
{
public:
    virtual void beforeStore(const llvm::StoreInst &instruction,
                             State &state,
                             const Environment &environment,
                             Domain *pointer,
                             Domain *value);

    virtual void afterStore(const llvm::StoreInst &instruction,
                            State &state,
                            const Environment &environment,
                            Domain *pointer,
                            Domain *value);
};

} // namespace Canal

#endif // LIBCANAL_CALLBACKS_H
