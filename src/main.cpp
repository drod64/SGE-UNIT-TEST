#include <TEST/unit-tests/pector_tester.h>
#include <iostream>

int main()
{
    test::pector_tester pUnitTest;
    pUnitTest.run();

    std::cout << "Unit test passed!\n";
    return 0;
}