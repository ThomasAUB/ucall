#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ucall.hpp"

TEST_CASE("basic ucall tests") {

    int ic1;
    ucall::Callable<void(int&)> c1(
        [&] (int& i) {
            ic1 = i;
            std::cout << "from closure lambda " << i++ << std::endl;
        }
    );

    static int ic2;
    ucall::Callable<void(int&)> c2(
        +[] (int& i) {
            ic2 = i;
            std::cout << "from basic lambda " << i++ << std::endl;
        }
    );

    struct Obj {
        void test(int& i) {
            mI = i;
            std::cout << "from member function " << i++ << std::endl;
        }
        int mI;
    };

    Obj inst;
    ucall::Callable<void(int&)> c3(&Obj::test, inst);
    ucall::Callable<void(int&)> c4(&Obj::test, &inst);

    int i = 22;
    c1(i);
    CHECK(ic1 == 22);
    c2(i);
    CHECK(ic2 == 23);
    c3(i);
    CHECK(inst.mI == 24);
    c4(i);
    CHECK(inst.mI == 25);
}

#include <functional>

TEST_CASE("ucall vs std::function") {

    ucall::Callable<void(int&)> c1(
        [&] (int& i) {
            i++;
        }
    );

    std::function<void(int&)> f1(
        [&] (int& i) {
            i++;
        }
    );

    ucall::Callable<void(int&)> c2(
        +[] (int& i) {
            i++;
        }
    );

    std::function<void(int&)> f2(
        +[] (int& i) {
            i++;
        }
    );

    auto getMicros = [] () {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
            );
        };

    constexpr int cycles = 1'000'000;
    int v = 0;

    auto start = getMicros();
    for (int i = 0; i < cycles; i++) {
        c1(v);
        c2(v);
    }
    auto end = getMicros();

    const auto callableDuration = end - start;

    start = getMicros();
    for (int i = 0; i < cycles; i++) {
        f1(v);
        f2(v);
    }
    end = getMicros();

    const auto stdFunctionDuration = end - start;

    //std::cout << "callable took " << callableDuration << std::endl;
    //std::cout << "std::function took " << stdFunctionDuration << std::endl;

    CHECK(stdFunctionDuration / callableDuration >= 2);
}

