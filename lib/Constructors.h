#ifndef LIBCANAL_CONSTRUCTORS_H
#define LIBCANAL_CONSTRUCTORS_H

namespace llvm {
class Type;
class Constant;
struct fltSemantics;
} // namespace llvm

namespace Canal {

class Domain;
class Environment;

class Constructors
{
protected:
    const Environment &mEnvironment;

public:
    Constructors(const Environment &environment);

    Domain *create(const llvm::Type &type) const;

    Domain *create(const llvm::Constant &value) const;

    static const llvm::fltSemantics *
        getFloatingPointSemantics(const llvm::Type &type);
};

} // namespace Canal

#endif // LIBCANAL_CONSTRUCTORS_H
