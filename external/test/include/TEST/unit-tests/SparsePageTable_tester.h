#ifndef SPARSE_PAGE_TABLE_TESTER_H
#define SPARSE_PAGE_TABLE_TESTER_H
#include <cassert>
#include <iostream>
#include <TEST/containers/SparsePageTable.h>

namespace test {
class SparsePageTable_tester {
public:
    void run();

private:
    void all_in_one();
}; // SparsePageTable_tester
} // namespace test

#endif // SPARSE_PAGE_TABLE_TESTER_H