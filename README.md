![build status](https://github.com/ThomasAUB/ucall/actions/workflows/build.yml/badge.svg)
[![License](https://img.shields.io/github/license/ThomasAUB/ucall)](LICENSE)

# uCall

Lightweight, type-erased wrapper for C++ callable entity, such as function pointers, lambda expression, or member functions.
It's designed as an alternative to `std::function` in contexts where heap allocation is not advised.

- single header file
- no heap allocation

## Example

```cpp
#include "ucall.hpp"

void freeFunction(int, bool) {
    // ...
}

void foo() {

    // free function
    ufunc::Callable<void(int, bool)> freeCallable(freeFunction);

    freeCallable(55, true);
    
    // capturing lambda
    bool b = true;
    ucall::Callable<void(int&)> capture(
        [&] (int& i) {
            b ? i++ : i--;
        }
    );

    int i = 0;
    capture(i);

    // non-capturing lambda
    ucall::Callable<int(std::string_view)> nonCapturing(
        +[] (std::string_view) {
            // ...
            return 0;
        }
    );
    
    auto result = nonCapturing("hello");

    // member function
    struct Object {
        void test() {
            // ...
        }
    };

    Object o;

    ucall::Callable objRef(&Object::test, o);
    ucall::Callable objPtr(&Object::test, &o);

    objRef();
    objPtr();

}

```