#include <TEST/unit-tests/SparseSet_tester.h>

void test::SparseSet_tester::run()
{
    std::cout << "Starting SparseSet unit test...\n";
    this->all_in_one();
    std::cout << "SparseSet unit test passed...\n\n";
}

void test::SparseSet_tester::all_in_one()
{
    sge::SparseSet<sge::Entity, sge::EntityIndexExtractor> sparseSet;

    // Create two dummy entities.
    sge::Entity testEntity1 = static_cast<sge::Entity>(1'048'576);
    sge::Entity testEntity2 = static_cast<sge::Entity>(1'048'577);

    // Insert first entity (ensure dense index is synced).
    sge::size_type denseIndex = sparseSet.insert(testEntity1);
    assert(denseIndex == 0);
    assert(sparseSet.index(testEntity1) == denseIndex);
    assert(sparseSet.contains(testEntity1));
    assert(!sparseSet.contains(testEntity2));

    // Insert second entity.
    sparseSet.insert(testEntity2);
    assert(sparseSet.contains(testEntity2));

    // Test swap logic (ensure keys are swapped in sparse and dense container).
    sparseSet.swapKeys(testEntity1, testEntity2);
    assert(sparseSet.getKeys()[0] == testEntity2);
    sge::size_type switchedDenseIndex = sparseSet.index(testEntity1);
    assert(switchedDenseIndex == 1);

    // Test erasure logic.
    sparseSet.erase(testEntity1, [](auto to, auto from){
        assert(to == 0);
        assert(from == 1);
    });
    assert(!sparseSet.contains(testEntity1));

    // Test duplicate insertion.
    sge::size_type duplicate1 = sparseSet.insert(testEntity1);
    sge::size_type duplicate2 = sparseSet.insert(testEntity1);
    assert(duplicate1 == duplicate2);
    assert(sparseSet.size() == 2);

    // Test clearing of sparse set.
    sparseSet.clear();
    assert(sparseSet.size() == 0);
    assert(!sparseSet.contains(testEntity1));
    assert(!sparseSet.contains(testEntity2));
}