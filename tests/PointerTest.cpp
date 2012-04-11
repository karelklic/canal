#include "../lib/Pointer.h"
#include <cassert>

int main(int argc, char **argv)
{
    // Test empty abstract pointers.
    Canal::Pointer::InclusionBased a, b;
    assert(a.mTargets.size() == 0);
    assert(a == b);
    a.merge(b);
    assert(a.mTargets.size() == 0);

    // One of the pointers points to some abstract value.
    //a.mTargets.insert(Canal::Pointer::Target());
    //assert(a != b);

    // Test clone with the abstract value.
    Canal::Pointer::InclusionBased *aa = a.clone();
    assert(a == *aa);
    // Check that the enumerations are separate instances.
    assert(*a.mTargets.begin() != *aa->mTargets.begin());
}
