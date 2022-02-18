#include <iostream>

//#include "ProxyService.h"

#define DEBUG_MODE
#ifdef DEBUG_MODE
#include "gtest/gtest.h"
#endif


#ifndef DEBUG_MODE
int main() {
    int port = 12345;
    ProxyService * ps = new ProxyService(port);
    ps->run();
}
#else

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif