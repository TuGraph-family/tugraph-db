//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\type_traits.h.
 *
 * \brief   Declares the type traits.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once
#include <functional>
#include <memory>
#include <type_traits>
#include <string>

#if (defined(__clang__) && __clang_major__ > 5)
#elif (defined(__GNUC__) && __GNUC__ < 5)
#define is_trivially_copyable has_trivial_copy_constructor
#endif

#if !defined(_MSC_VER) && __cplusplus < 201402L
namespace std {
template <typename T, typename... Ts>
unique_ptr<T> make_unique(Ts &&...ts) {
    return unique_ptr<T>(new T(forward<Ts>(ts)...));
}
}  // namespace std
#endif

#if (defined(__GNUC__))
#define _F_LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#define _F_UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)
#else
#define _F_LIKELY(condition) static_cast<bool>(condition)
#define _F_UNLIKELY(condition) static_cast<bool>(condition)
#endif

namespace fma_common {
/*!
 * \def HAS_MEM_FUNC(func, name)
 *
 * \brief   A type trait that checks whether type T has member function called
 *          func that has a signature Sign.
 */
#define DEFINE_HAS_MEM_FUNC_TEMPLATE(func_name, struct_name)        \
    template <typename T, typename Sign>                            \
    struct struct_name {                                            \
        typedef char yes[1];                                        \
        typedef char no[2];                                         \
        template <typename U, U>                                    \
        struct type_check;                                          \
        template <typename _1>                                      \
        static yes &chk(type_check<Sign, &_1::func_name> *);        \
        template <typename>                                         \
        static no &chk(...);                                        \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes); \
    }

template <typename T>
class _has_std_to_string_ {
    template <typename U, typename _ = decltype(std::to_string(std::declval<U>()))>
    static std::true_type test(int);

    template <typename U>
    static std::false_type test(...);

 public:
    static bool const value = decltype(test<T>(0))::value;
};

/*!
 * \struct  _user_friendly_static_assert_
 *
 * \brief   Helper template to generate user friendly error message in case
 *          a type is not supported.
 */
template <bool v, typename T_>
struct _user_friendly_static_assert_ {
    static_assert(v, "Assertion failed <see below for more information>");
    static bool const value = v;
};

/*!
 * \def DISABLE_COPY(class_name)
 *
 * \brief   A macro that disable copy of class by declaring its copy
 *          constructor and assignment operator as deleted.
 *
 * \param   class_name  Name of the class.
 */
#define DISABLE_COPY(class_name)             \
    class_name(const class_name &) = delete; \
    class_name &operator=(const class_name &) = delete;

/*!
 * \def DISABLE_MOVE(class_name)
 *
 * \brief   A macro that disable move of class by declaring its move
 *          constructor and move assignment operator as deleted.
 *
 * \param   class_name  Name of the class.
 */
#define DISABLE_MOVE(class_name)        \
    class_name(class_name &&) = delete; \
    class_name &operator=(class_name &&) = delete;

template <typename T>
struct _Identity {
    typedef T type;
};

#define ENABLE_IF_VOID(T1, T2)  \
    template <typename T_ = T1> \
    typename std::enable_if<std::is_void<T_>::value, T2>::type

#define ENABLE_IF_NOT_VOID(T1, T2) \
    template <typename T_ = T1>    \
    typename std::enable_if<!std::is_void<T_>::value, T2>::type

#define TYPE_IF_VOID(T1, T2) typename std::enable_if<std::is_void<T1>::value, T2>::type

#define TYPE_IF_NOT_VOID(T1, T2) typename std::enable_if<!std::is_void<T1>::value, T2>::type

template <class T>
struct DummyTypeIfVoid {
    typedef T type;
};

template <>
struct DummyTypeIfVoid<void> {
    typedef int type;
};

#define DUMMY_TYPE_IF_VOID(T) typename DummyTypeIfVoid<T>::type

template <typename T, typename T1, typename... Ts>
struct IsType {
    static const bool value = std::is_same<T, T1>::value || IsType<T, Ts...>::value;
};

template <typename T, typename T1>
struct IsType<T, T1> {
    static const bool value = std::is_same<T, T1>::value;
};

template <typename T>
struct SizeOfType {
    inline static size_t value() { return sizeof(T); }
};

template <>
struct SizeOfType<void> {
    inline static size_t value() { return 0; }
};

template <class T>
struct EnableNonVoidValue {
    EnableNonVoidValue() {}
    explicit EnableNonVoidValue(const T &v) : value(v) {}
    operator const T &() const { return value; }
    operator T &() { return value; }

    T value;
};

template <>
struct EnableNonVoidValue<void> {};

template <typename T, int cacheLineSize = 64>
struct PadForCacheLine {
    PadForCacheLine() {}
    explicit PadForCacheLine(const T &v) : value(v) {}
    operator T &() { return value; }
    operator const T &() const { return value; }
    T value;
    char _dummyPadding[cacheLineSize - sizeof(T) % cacheLineSize]{};
};

namespace _detail {
template <size_t N>
struct Apply {
    template <typename F, typename T, typename... A>
    static inline auto apply(F &&f, T &&t, A &&...a)
        -> decltype(Apply<N - 1>::apply(::std::forward<F>(f), ::std::forward<T>(t),
                                        ::std::get<N - 1>(::std::forward<T>(t)),
                                        ::std::forward<A>(a)...)) {
        return Apply<N - 1>::apply(::std::forward<F>(f), ::std::forward<T>(t),
                                   ::std::get<N - 1>(::std::forward<T>(t)),
                                   ::std::forward<A>(a)...);
    }
};

template <>
struct Apply<0> {
    template <typename F, typename T, typename... A>
    static inline auto apply(F &&f, T &&, A &&...a)
        -> decltype(::std::forward<F>(f)(::std::forward<A>(a)...)) {
        return ::std::forward<F>(f)(::std::forward<A>(a)...);
    }
};

template <typename F, typename T>
inline auto ApplyTuple(F &&f, T &&t)
    -> decltype(_detail::Apply<::std::tuple_size<typename ::std::decay<T>::type>::value>::apply(
        std::forward<F>(f), std::forward<T>(t))) {
    return _detail::Apply<::std::tuple_size<typename ::std::decay<T>::type>::value>::apply(
        std::forward<F>(f), std::forward<T>(t));
}

template <typename T>
struct memfun_type {
    using type = void;
};

template <typename Ret, typename Class, typename... Args>
struct memfun_type<Ret (Class::*)(Args...) const> {
    using type = std::function<Ret(Args...)>;
};

template <typename F>
typename _detail::memfun_type<decltype(&F::operator())>::type LambdaToFunction(F const &func) {
    return func;
}

template <size_t N>
struct Expand {
    template <typename T, typename D, typename... A>
    static inline auto apply(T &&t, D &&d, A &&...a)
        -> decltype(Expand<N - 1>::apply(::std::forward<T>(t), ::std::forward<D>(d),
                                         ::std::get<N - 1>(::std::forward<T>(t)),
                                         ::std::forward<A>(a)...)) {
        return Expand<N - 1>::apply(::std::forward<T>(t), ::std::forward<D>(d),
                                    ::std::get<N - 1>(::std::forward<T>(t)),
                                    ::std::forward<A>(a)...);
    }
};

template <>
struct Expand<0> {
    template <typename D, typename T, typename... A>
    static inline auto apply(T &&, D &&d, A &&...a)
        -> decltype(::std::make_tuple(std::forward<A>(a)..., std::forward<D>(d))) {
        return ::std::make_tuple(std::forward<A>(a)..., std::forward<D>(d));
    }
};

template <typename T, typename TUP>
auto ExpandTuple(TUP &&tup, T &&el)
    -> decltype(_detail::Expand<::std::tuple_size<typename std::decay<TUP>::type>::value>::apply(
        ::std::forward<TUP>(tup), ::std::forward<T>(el))) {
    return _detail::Expand<::std::tuple_size<typename std::decay<TUP>::type>::value>::apply(
        ::std::forward<TUP>(tup), ::std::forward<T>(el));
}
}  // namespace _detail
}  // namespace fma_common
