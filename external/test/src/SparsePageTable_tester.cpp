#include <TEST/unit-tests/SparsePageTable_tester.h>


void test::SparsePageTable_tester::run()
{
    std::cout << "Starting SparsePageTable unit test...\n";
    this->all_in_one();
    std::cout << "SparsePageTable unit test passed...\n\n";
}

void test::SparsePageTable_tester::all_in_one()
{
    sge::SparsePageTable<size_t> sparse;

    for (size_t i = 0; i < 100; ++i)
    {
        sparse.set(i, 100);
    }

    for (size_t i = 0; i < 100; ++i)
    {
        assert(sparse.get(i) == 100);
    }
    assert(sparse.get(300) == sparse.TOMBSTONE);
    assert(sparse.allocatedPageCount() == 1);

    sparse.set(5, 0);

    sge::SparsePageTable<size_t> sparse2(std::move(sparse));
    assert(sparse2.contains(56));
    assert(sparse2.allocatedPageCount() == 1);
    assert(sparse.allocatedPageCount() == 0);
    assert(sparse.pageCount() == 0);
    
    sparse2.erase(5);
    sparse = std::move(sparse2);

    assert(!sparse.contains(5));

    sparse.release();
    assert(sparse.pageCount() == 0);

    sparse.set(1000000, 1);
    assert(sparse.get(1000000) == 1);
    assert(sparse.allocatedPageCount() == 1);
    assert(sparse.pageCount() == 977);

    sparse.clear();
    assert(!sparse.contains(1000000));
    assert(!sparse.contains(0));
    assert(!sparse.contains(1));
    assert(!sparse.contains(34));
}

