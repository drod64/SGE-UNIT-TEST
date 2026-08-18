#ifndef TEST_PECTOR_TESTER_H
#define TEST_PECTOR_TESTER_H
#include <string>
#include <iostream>
#include <TEST/containers/pector.h>

namespace test {
class pector_tester {
public:
    void run();

private:
    /**
     * Tests the pector for push_back() and accessing.
     */
    void appendAndAccess();

    /**
     * Tests the pector for reserve() and resize() functionality.
     */
    void reserveAndResize();

    /**
     * Tests the pector's iterators.
     */
    void iterator();

    /**
     * Tests the pector's const iterators.
     */
    void const_iterator();

    /**
     * Tests the pector's reverse iterators.
     */
    void reverse_iterator();

    /**
     * Tests the pector's move semantics.
     */
    void moveAndSwapSemantics();

}; // class pector_tester
}; // namespace test

#endif // TEST_PECTOR_TESTER_H