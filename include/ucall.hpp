/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MIT License                                                                     *
 *                                                                                 *
 * Copyright (c) 2024 Thomas AUBERT                                                *
 *                                                                                 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy    *
 * of this software and associated documentation files (the "Software"), to deal   *
 * in the Software without restriction, including without limitation the rights    *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is           *
 * furnished to do so, subject to the following conditions:                        *
 *                                                                                 *
 * The above copyright notice and this permission notice shall be included in all  *
 * copies or substantial portions of the Software.                                 *
 *                                                                                 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
 * SOFTWARE.                                                                       *
 *                                                                                 *
 * github : https://github.com/ThomasAUB/ucall                                     *
 *                                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

#include <new>
#include <cstddef>
#include <stdint.h>

namespace ucall {

    template<typename return_t, typename ... args_t> struct Callable;

    template<typename return_t, typename ...args_t>
    struct Callable<return_t(args_t...)> {

        template<typename callable_t>
        Callable(callable_t&& c) {
            make(c);
        }

        template<typename T>
        Callable(T& c, return_t(T::* f)(args_t...)) {

            //make([&c, f] (args_t... a) { return (c.*f)(a...); });
            make([&] (args_t... a) { return (c.*f)(a...); });
        }

        return_t operator()(args_t... args) {
            return (*mInterface)(args...);
        }

    private:

        struct Interface { virtual return_t operator()(args_t...) = 0; };

        template<typename c_t>
        struct Impl : Interface {
            c_t& mC;
            Impl(c_t& c) : mC(c) {}
            return_t operator()(args_t... a) override { return mC(a...); }
        };

        template<typename callable_t>
        void make(callable_t&& c) {
            static_assert(
                sizeof(callable_t) <= sizeof(mStorage),
                "Storage too small"
            );
            mInterface = ::new(mStorage) Impl<callable_t>(c);
            //mInterface = ::new(mStorage) callable_t(c);
        }


        alignas(std::max_align_t) uint8_t mStorage[sizeof(uintptr_t) * 3];

        Interface* mInterface = nullptr;
    };

}
