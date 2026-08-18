#include <TEST/unit-tests/pector_tester.h>
#include <iostream>
#include <algorithm>

void test::pector_tester::run()
{
    std::cout << "Starting pector_tester...\n";
    this->appendAndAccess();

    this->reserveAndResize();

    this->iterator();

    this->const_iterator();

    this->moveAndSwapSemantics();

    std::cout << "pector_test unit tests passed...\n\n";
}

void test::pector_tester::appendAndAccess()
{
    sge::pector<int> p;

    for (size_t i = 0; i < 40; ++i)
    {
        p.push_back(i);
    }

    assert(p.size() == 40);

    assert(p.capacity() == 1024);

    assert(!p.empty());
}

void test::pector_tester::reserveAndResize()
{
    sge::pector<std::string> p;

    p.resize(9000);
    assert(p[0] == "");
    assert(p[8999] == "");
    assert(p.capacity() == 9216);
    p.release();

    p.resize(10);
    p.reserve(13045);
    assert(p.capacity() == 13312);
    
    p.resize(100);

    assert(p.size() == 100);
    assert(p.capacity() == 13312);

    p.release();
    assert(p.size() == 0);
    assert(p.capacity() == 0);
    assert(p.empty());

    p.reserve(9000);
    assert(p.capacity() == 9216);

    p.release();
    p.resize(100, "bruh");
    assert(p[0] == "bruh");
    assert(p[49] == "bruh");
    assert(p[99] == "bruh");

    p.pop_back();
    assert(p.size() == 99);
}

void test::pector_tester::iterator()
{
    // =========================================================================
    // Edge Case 1: The Completely Empty State
    // =========================================================================
    {
        sge::pector<std::string> p;
        assert(p.begin() == p.end());
        
        // Ensure range-based for loops exit immediately without a single step.
        size_t loopCount = 0;
        for (const auto& str : p) {
            (void)str;
            ++loopCount;
        }
        assert(loopCount == 0);
    }

    // =========================================================================
    // Edge Case 2: Exact Page Boundary Alignment (No Dummy Page)
    // =========================================================================
    {
        sge::pector<std::string> p;
        p.reserve(p.ELEMENTS_PER_PAGE);

        for (size_t i = 0; i < p.ELEMENTS_PER_PAGE; ++i)
        {
            p.push_back("item_" + std::to_string(i));
        }

        auto it = p.begin();
        it += p.ELEMENTS_PER_PAGE; // Jump exactly to the end edge.
        assert(it == p.end());
        
        // Verifying distance math.
        assert((p.end() - p.begin()) == static_cast<ptrdiff_t>(p.ELEMENTS_PER_PAGE));
        assert((p.end() - p.begin()) == static_cast<ptrdiff_t>(p.size()));
    }

    // =========================================================================
    // Edge Case 3: Sequential & Multi-Page Boundary Jumps (operator+=)
    // =========================================================================
    {
        sge::pector<std::string> p;
        // Fill across two full pages into a third page (e.g., 2200 items).
        for (size_t i = 0; i < 2200; ++i) {
            p.push_back("val_" + std::to_string(i));
        }

        // Test sequential step crossing exactly at the Page 0 -> Page 1 border.
        auto it = p.begin();
        it += (p.ELEMENTS_PER_PAGE - 1); // Sit at index 1023 (last item of Page 0).
        assert(*it == "val_1023");
        
        ++it; // This step must trigger the page-flip branch seamlessly.
        assert(*it == "val_1024"); // First item of Page 1.

        // Test a massive random-access jump skipping over an entire page block.
        auto jumpIt = p.begin();
        jumpIt += 2118; // Jump from Page 0 into Page 2.
        assert(*jumpIt == "val_2118");

        // Test ++operator/--operator on end() edge.
        auto lastValid = p.end() - 1;
        ++lastValid;
        assert(lastValid == p.end());

        --lastValid;
        assert(*lastValid == "val_2199");
    }

    // =========================================================================
    // Edge Case 4: Reverse Page Traversal (operator-=)
    // =========================================================================
    {
        sge::pector<std::string> p;
        for (size_t i = 0; i < 2200; ++i) {
            p.push_back("val_" + std::to_string(i));
        }

        // Sit precisely at the first element of Page 1 (index 1024).
        auto it = p.begin();
        it += p.ELEMENTS_PER_PAGE; 
        assert(*it == "val_1024");

        --it; // This decrement must safely step back across the page boundary wall.
        assert(*it == "val_1023");

        // Test a massive relative subtraction backtracking leap.
        auto backIt = p.end();
        --backIt; // Sit at index 1099
        backIt -= 1110; // Jump backward into a completely different page chunk.
        assert(*backIt == "val_1089");
    }

    // =========================================================================
    // Edge Case 5: Relational Sorting Integration (std::sort & <=> Check)
    // =========================================================================
    {
        sge::pector<std::string, 9> p;
        // Seed strings in reverse order across multiple page chunks.
        for (int i = 1200; i >= 0; --i) {
            p.push_back("padded_num_" + std::to_string(10000 + i)); 
        }

        // std::sort tests random-access spacing, less-than pivots, and bracket swapping.
        std::sort(p.begin(), p.end());

        assert(std::is_sorted(p.begin(), p.end()));
        assert(*p.begin() == "padded_num_10000");
        assert(*(p.end() - 1) == "padded_num_11200");
    }

    // =========================================================================
    // Edge Case 6: Memory Mutations (Clear & Resize Invalidation)
    // =========================================================================
    {
        sge::pector<std::string> p;
        p.push_back("safe_string");
        
        auto oldIt = p.begin();
        assert(*oldIt == "safe_string");

        // calling clear() should cleanly detach active storage trackers.
        p.clear();
        assert(p.begin() == p.end());
        
        // Re-filling after clear to verify factory baseline initialization.
        p.push_back("new_string");
        assert(*p.begin() == "new_string");
    }
}

void test::pector_tester::const_iterator()
{
    sge::pector<std::string> p;
    p.resize(9000, "default");

    for (auto it = p.cbegin() ; it != p.cend(); ++it)
    {
        assert(*it == "default");
    }

    auto it_jump = p.cbegin();
    it_jump += p.size();

    assert(it_jump == p.cend());
    assert(it_jump == p.end());
}

void test::pector_tester::reverse_iterator()
{
    sge::pector<std::string> p;
    for (size_t i = 0; i < 2200; ++i)
    {
        p.push_back("val_" + std::to_string(i));
    }

    size_t last = p.size() - 1;
    for (auto it = p.rbegin(); it != p.rend(); ++it)
    {
        assert(*it == "val_" + std::to_string(last));
    }
}

void test::pector_tester::moveAndSwapSemantics()
{
    sge::pector<std::string> p;
    p.push_back("Hello");

    assert(p[0] == "Hello");

    std::string varString = "Bye";
    p.push_back(varString);

    assert(p[1] == varString);

    std::string* pointer_to_p = &p[1];

    sge::pector<std::string> p2(std::move(p));
    
    std::string* pointer_to_p2 = &p2[1];

    assert(pointer_to_p == pointer_to_p2);

    assert(p.empty());
    assert(p2[0] == "Hello");

    p = std::move(p2);

    assert(p[0] == "Hello");
    p.clear();
    assert(p.empty());
    assert(p.capacity() == 1024);

    assert(p2.empty());
    assert(p2.capacity() == 0);
    p2.clear();

    p2.emplace_back("blud");
    assert(p2[0] == "blud");

    p.emplace_back("Hello");
    p.push_back("Bye");
    p.swapElements(0, 1);
    assert(p[0] == "Bye");
    assert(p[1] == "Hello");
}