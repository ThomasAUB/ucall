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
#include <utility>
#include <cstddef>
#include <type_traits>

namespace ucall {

    template<typename> struct Callable;

    template<typename return_t, typename ...args_t>
    struct Callable<return_t(args_t...)> {

        Callable() = default;

        template<typename callable_t,
            typename = std::enable_if_t<
            std::is_invocable_r_v<return_t, callable_t, args_t...> &&
            !std::is_same_v<std::decay_t<callable_t>, Callable>>
            >
            Callable(callable_t&& c) {
            using decayed = std::decay_t<callable_t>;
            static_assert(
                sizeof(decayed) <= storage_size,
                "Callable object too large for internal storage."
                );
            static_assert(
                alignof(decayed) <= storage_align,
                "Callable object alignment too strict for internal storage."
                );
            ::new(mStorage) decayed(std::forward<callable_t>(c));
            mOps = &ops_table<decayed>;
        }

        template<typename T>
        Callable(return_t(T::* f)(args_t...), T& obj)
            : Callable([&obj, f] (args_t... args) -> return_t { return (obj.*f)(args...); }) {}

        template<typename T>
        Callable(return_t(T::* f)(args_t...), T* obj)
            : Callable([obj, f] (args_t... args) -> return_t { return (obj->*f)(args...); }) {}

        Callable(const Callable& other) noexcept {
            if (other.mOps) {
                other.mOps->copy(other.mStorage, this->mStorage);
                mOps = other.mOps;
            }
        }

        Callable(Callable&& other) noexcept {
            if (other.mOps) {
                other.mOps->copy(other.mStorage, this->mStorage);
                mOps = other.mOps;
                other.mOps->destroy(other.mStorage);
                other.mOps = nullptr;
            }
        }

        ~Callable() {
            if (mOps) {
                mOps->destroy(mStorage);
            }
        }

        return_t operator()(args_t... args) const {
            if (!mOps) { while (true); }
            return mOps->invoke(mStorage, std::forward<args_t>(args)...);
        }

        Callable& operator=(Callable&& other) noexcept {
            if (this != &other) {
                if (mOps) { mOps->destroy(mStorage); }
                if (other.mOps) {
                    other.mOps->copy(other.mStorage, this->mStorage);
                    mOps = other.mOps;
                    other.mOps->destroy(other.mStorage);
                    other.mOps = nullptr;
                }
                else {
                    mOps = nullptr;
                }
            }
            return *this;
        }

        Callable& operator=(const Callable& other) noexcept {
            if (this != &other) {
                if (mOps) { mOps->destroy(mStorage); }
                if (other.mOps) {
                    other.mOps->copy(other.mStorage, this->mStorage);
                    mOps = other.mOps;
                }
                else {
                    mOps = nullptr;
                }
            }
            return *this;
        }

        operator bool() const {
            return (mOps != nullptr);
        }

    private:

        struct Operations {
            return_t(*invoke)(const std::byte*, args_t...);
            void (*destroy)(std::byte*);
            void (*copy)(const std::byte*, std::byte*);
        };

        template<typename callable_t>
        static return_t invoke_impl(const std::byte* storage, args_t... args) {
            const auto* callable = reinterpret_cast<const callable_t*>(storage);
            return (*callable)(std::forward<args_t>(args)...);
        }

        template<typename callable_t>
        static void destroy_impl(std::byte* storage) {
            auto* callable = reinterpret_cast<callable_t*>(storage);
            callable->~callable_t();
        }

        template<typename callable_t>
        static void copy_impl(const std::byte* src, std::byte* dst) {
            const auto* src_callable = reinterpret_cast<const callable_t*>(src);
            ::new(dst) callable_t(*src_callable);
        }

        template<typename callable_t>
        static constexpr Operations ops_table = {
            &invoke_impl<callable_t>,
            &destroy_impl<callable_t>,
            &copy_impl<callable_t>
        };

        static constexpr size_t storage_size = 4 * sizeof(uintptr_t);
        static constexpr size_t storage_align = alignof(std::max_align_t);

        alignas(storage_align) std::byte mStorage[storage_size];
        const Operations* mOps = nullptr;
    };

}
