#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ucall.hpp"

struct Obj {

    void test(int i) {
        std::cout << "from member function " << i << std::endl;
    }

};

TEST_CASE("basic ucall tests") {

    ucall::Callable<void(int)> c1(
        [&] (int i) {
            std::cout << "from closure lambda " << i << std::endl;
        }
    );
    ucall::Callable<void(int)> c2(
        +[] (int i) {
            std::cout << "from basic lambda " << i << std::endl;
        }
    );

    Obj inst;
    ucall::Callable<void(int)> c3(inst, &Obj::test);

    c1(55);
    c2(78);
    c3(789);
}

