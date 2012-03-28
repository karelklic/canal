#include "../lib/AbstractPointer.h"
#include <cassert>

int main(int argc, char **argv)
{
    // Test empty abstract pointers.
    AbstractPointer a, b;
    assert(a.mTargets.size() == 0);
    assert(a == b);
    a.merge(b);
    assert(a.mTargets.size() == 0);

    // One of the pointers points to some abstract value.
    AbstractPointer *value = new AbstractPointer();
    a.mTargets.insert(value);
    assert(a != b);

    // Test clone with the abstract value.
    AbstractPointer *aa = a.clone();
    assert(a == *aa);
    // Check that the enumerations are separate instances.
    assert(*a.mTargets.begin() != *aa->mTargets.begin());
}
