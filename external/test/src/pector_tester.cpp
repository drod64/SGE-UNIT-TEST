#include <TEST/unit-tests/pector_tester.h>
#include <iostream>
#include <algorithm>

void test::pector_tester::run()
{
    this->appendAndAccess();

    this->reserveAndResize();

    this->iterator();

    this->moveSemantics();
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
}

void test::pector_tester::iterator()
{
    sge::pector<std::string> p;

    p.resize(1505, "Hi");

    auto itb = p.begin();
    auto ite = p.end();

    auto dist = ite - itb;

    assert(dist == 1505);

    sge::pector<int> sortPector;
    for (int i = 0; i < 100; ++i)
    {
        sortPector.push_back(100 - i);
    }

    std::sort(sortPector.begin(), sortPector.end());

    assert(sortPector[0] == 1);
    assert(sortPector[49] == 50);
    assert(sortPector[99] == 100);
}

void test::pector_tester::moveSemantics()
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
}