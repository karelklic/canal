#define DEBUG
#include "lib/SuperPtr.h"
#include "lib/Utils.h"
#include <iostream>

using namespace Canal;

static void intTest() {
    typedef SuperPtr<int> type;
    type tmp1(0);
    const type& tmp1ref(tmp1);

    CANAL_ASSERT(*tmp1ref == 0);
    CANAL_ASSERT(*tmp1 == 0); //Should not force object copy
    CANAL_ASSERT(tmp1ref.getCounter() == 1);

    type tmp2(tmp1);
    const type& tmp2ref(tmp2);
    CANAL_ASSERT(*tmp1ref == 0);
    CANAL_ASSERT(*tmp2ref == 0);

    CANAL_ASSERT(tmp1ref.getCounter() == 2);
    CANAL_ASSERT(tmp2ref.getCounter() == 2);

    CANAL_ASSERT(*tmp2 == 0); //Forces object copy
    CANAL_ASSERT(tmp1ref.getCounter() == 1);
    CANAL_ASSERT(tmp2ref.getCounter() == 1);

    CANAL_ASSERT(tmp1ref == 0);
    CANAL_ASSERT(tmp2ref == 0);
    CANAL_ASSERT(tmp1ref == tmp2ref);
}


int main() {
    intTest();
}
