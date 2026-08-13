#include <TEST/unit-tests/pector_tester.h>
#include <iostream>

void test::pector_tester::run()
{
    this->appendAndAccess();

    this->reserveAndResize();

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

    // p.resize(10);
    p.reserve(13045);
    assert(p.capacity() == 13312);
    
    p.resize(100);
    
    assert(p.size() == 100);
    assert(p.capacity() == 13312);

    p.release();

    assert(p.size() == 0);
    assert(p.capacity() == 0);
    assert(p.empty());
}

void test::pector_tester::iterator()
{
    sge::pector<std::string> p;

    p.resize(1505);

    assert(p.size() == 1505);
    assert(p.capacity() == 2048);

    // for (auto it = p.begin(); it != p.end(); ++it)
    // {
    //     std::cout << *it << '\n';
    // }
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