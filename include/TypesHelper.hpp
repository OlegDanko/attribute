#pragma once
#include <cstddef>
#include <type_traits>
#include <tuple>

template <typename... Ts>
struct types {
    using tpl = std::tuple<Ts...>;
    static constexpr size_t count = std::tuple_size<tpl>();
};

template<typename, typename>
struct cat_types;
template<typename T, typename ...Ts>
struct cat_types<T, types<Ts...>> {
    using types_ = types<T, Ts...>;
};
template<typename ...Ts1, typename ...Ts2>
struct cat_types<types<Ts1...>, types<Ts2...>> {
    using types_ = types<Ts1..., Ts2...>;
};

template<typename, size_t>
struct types_tail;
template<typename T, typename ... Ts, size_t I>
struct types_tail<types<T, Ts...>, I> {
    using types_ = typename std::conditional<I == 0,
                                             types<T, Ts...>,
                                             typename types_tail<types<Ts...>, I - 1>::types_>::type;
};
template<size_t I>
struct types_tail<types<>, I> {
    using types_ = types<>;
};

template<template<typename> typename, typename...>
struct type_apply;
template<template<typename> typename T, typename T1, typename...Ts>
struct type_apply<T, T1, Ts...> {
    using types_ = typename cat_types<T<T1>, typename type_apply<T, Ts...>::types_>::types_;
};
template<template<typename> typename T>
struct type_apply<T> {
    using types_ = types<>;
};
template<template<typename> typename T, typename ...Ts>
struct type_apply<T, types<Ts...>> {
    using types_ = typename type_apply<T, Ts...>::types_;
};

template<typename, typename...>
struct is_unique;
template<typename T>
struct is_unique<T> { static constexpr bool val = false; };
template<typename T, typename T1, typename ...Ts>
struct is_unique<T, T1, Ts...> { static constexpr bool val = (!std::is_same<T, T1>::value) && is_unique<T, Ts...>::val; };
template<typename T, typename... Ts>
static constexpr bool is_unique_v = is_unique<T, Ts...>::val;

template<typename, typename...>
struct is_present;
template<typename T>
struct is_present<T> { static constexpr bool val = false; };
template<typename T, typename T1, typename ...Ts>
struct is_present<T, T1, Ts...> { static constexpr bool val = (std::is_same<T, T1>::value) || is_present<T, Ts...>::val; };
template<typename T, typename... Ts>
static constexpr bool is_present_v = is_present<T, Ts...>::val;

template<typename ... Ts>
struct unique_types;
template<typename T>
struct unique_types<T> {
    using types_ = types<T>;
};
template<typename T, typename ...Ts>
class unique_types<T, Ts...> {
    using tail_types_ = typename unique_types<Ts...>::types_;
    using all_types = typename cat_types<T, tail_types_>::types_;
public:
    using types_ = typename std::conditional<is_unique<T, Ts...>::val,
                                            all_types,
                                            tail_types_>::type;
};
template<typename ...Ts>
struct unique_types<types<Ts...>> {
    using types_ = typename unique_types<Ts...>::types_;
};

template<std::size_t I>
struct size_ {
    static constexpr std::size_t val = I;
};

template<typename T, typename ...Ts>
struct index_of;
template<typename T>
struct index_of<T> {
    static constexpr size_t i = 0;
};
template<typename T, typename T1, typename ...Ts>
struct index_of<T, T1, Ts...> {
    static constexpr size_t i =
            std::conditional<std::is_same_v<T, T1>,
                             size_<0>,
                             size_<1 + index_of<T, Ts...>::i>>::type::val;
};
template<typename T, typename ...Ts>
struct index_of<T, types<Ts...>> {
    static constexpr size_t i = index_of<T, Ts...>::i;
};

template<typename T, typename ...Ts>
constexpr size_t index_of_v = index_of<T, Ts...>::i;

