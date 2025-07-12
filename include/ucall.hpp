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
            std::is_invocable_r_v<return_t, callable_t, args_t...>>
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
            mInterface = ::new(mStorage) Impl<decayed>(std::forward<callable_t>(c));
        }

        template<typename T>
        Callable(return_t(T::* f)(args_t...), T& obj)
            : Callable([&obj, f] (args_t... args) -> return_t { return (obj.*f)(args...); }) {}

        template<typename T>
        Callable(return_t(T::* f)(args_t...), T* obj)
            : Callable([obj, f] (args_t... args) -> return_t { return (obj->*f)(args...); }) {}

        Callable(Callable& other) noexcept {
            if (this != &other) {
                if (mInterface) { mInterface->~Interface(); }
                if (other.mInterface) {
                    other.mInterface->copy(this);
                }
                else {
                    mInterface = nullptr;
                }
            }
        }

        ~Callable() {
            if (mInterface) {
                mInterface->~Interface();
            }
        }

        return_t operator()(args_t... args) const {
            if (!mInterface) { while (true); }
            return (*mInterface)(std::forward<args_t>(args)...);
        }

        Callable& operator=(Callable&& other) noexcept {
            if (this != &other) {
                if (mInterface) { mInterface->~Interface(); }
                if (other.mInterface) {
                    other.mInterface->copy(this);
                }
                else {
                    mInterface = nullptr;
                }
            }
            return *this;
        }

        Callable& operator=(const Callable& other) noexcept {
            if (this != &other) {
                if (mInterface) { mInterface->~Interface(); }
                if (other.mInterface) {
                    other.mInterface->copy(this);
                }
                else {
                    mInterface = nullptr;
                }
            }
            return *this;
        }

        operator bool() const {
            return (mInterface != nullptr);
        }

    private:

        struct Interface {
            virtual return_t operator()(args_t...) const = 0;
            virtual void copy(Callable* c) = 0;
            virtual ~Interface() = default;
        };

        template<typename callable_t>
        struct Impl final : Interface {

            callable_t mCallable;

            template<typename... targs_t>
            Impl(targs_t&&... args) : mCallable(std::forward<targs_t>(args)...) {}

            void copy(Callable* c) override {
                new (c->mStorage) Impl(std::move(mCallable));
                c->mInterface = reinterpret_cast<Interface*>(c->mStorage);
            }

            return_t operator()(args_t... args) const override {
                return mCallable(std::forward<args_t>(args)...);
            }

        };

        static constexpr size_t storage_size = 4 * sizeof(uintptr_t);
        static constexpr size_t storage_align = alignof(std::max_align_t);

        alignas(storage_align) std::byte mStorage[storage_size];

        Interface* mInterface = nullptr;
    };

}
