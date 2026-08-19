#include <TEST/unit-tests/pector_tester.h>
#include <TEST/unit-tests/SparsePageTable_tester.h>
#include <TEST/unit-tests/SparseSet_tester.h>

int main()
{
    test::pector_tester pUnitTest;
    pUnitTest.run();

    test::SparsePageTable_tester sparseUnitTest;
    sparseUnitTest.run();

    test::SparseSet_tester sparseSetUnitTest;
    sparseSetUnitTest.run();
    return 0;
}