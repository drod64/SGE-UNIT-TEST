#include <TEST/unit-tests/pector_tester.h>
#include <TEST/unit-tests/SparsePageTable_tester.h>

int main()
{
    test::pector_tester pUnitTest;
    pUnitTest.run();

    test::SparsePageTable_tester sparseUnitTest;
    sparseUnitTest.run();
    return 0;
}