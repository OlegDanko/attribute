#pragma once

#include <type_traits>
#include <utility>


template<typename ...Ts>
struct TypesChain;

template<>
struct TypesChain<> {};

template<typename T>
struct TypesChain<T> {
    T arg;

    TypesChain() = default;
    TypesChain(const T& a) : arg(a) {}
    TypesChain(T&& a) : arg{std::move(a)} {}
    TypesChain(TypesChain&& chain) : arg{std::move(chain.arg)} {}
    TypesChain(const TypesChain& chain) : arg{chain.arg} {}

    template<typename A>
    A& get() {
        return arg;
    }
    template<typename A>
    const A& get() const {
        return arg;
    }
};

template<typename T, typename ...Ts>
struct TypesChain<T, Ts...> : TypesChain<T> {
    using Base_t = TypesChain<T>;
    TypesChain<Ts...> chain;

    TypesChain() = default;
    TypesChain(const T& a, const Ts&... args) : Base_t(a), chain(args...) {}
    TypesChain(T&& a, Ts&&... args) : Base_t(std::move(a)), chain(std::move(args)...) {}

    TypesChain(TypesChain&& that)
        : Base_t(std::move(static_cast<Base_t>(that).arg)),
          chain(std::move(that.chain)) {}

    TypesChain(const TypesChain& that)
        : Base_t(static_cast<Base_t>(that)),
          chain(std::move(that.chain)) {}

    template<typename A>
    A& get() {
        if constexpr (std::is_same<A, T>::value) {
            return Base_t::arg;
        } else {
            return chain.template get<A>();
        }
    }
    template<typename A>
    const A& get() const {
        if constexpr (std::is_same<A, T>::value) {
            return Base_t::arg;
        } else {
            return chain.template get<A>();
        }
    }
};

template<typename ...Ts>
TypesChain<Ts...> make_types_chain(Ts&& ... args) {
    return TypesChain<Ts...>(std::move(args)...);
}

template<> inline TypesChain<> make_types_chain<>() {
    return {};
}

template<template<typename >typename Arg, typename ...Ts>
struct SubTypesChain;

template<template<typename >typename Arg>
struct SubTypesChain<Arg> {};

template<template<typename >typename Arg, typename ...Ts>
struct SubTypesChain : private TypesChain<Arg<Ts>...> {
    SubTypesChain() = default;
    SubTypesChain(Arg<Ts>&&...args) : TypesChain<Arg<Ts>...>(std::move(args)...) {}
    SubTypesChain(SubTypesChain&& that) : TypesChain<Arg<Ts>...>(std::move(that)) {}
    SubTypesChain(const SubTypesChain& that) : TypesChain<Arg<Ts>...>(that) {}

    template<typename T>
    Arg<T>& get() {
        return TypesChain<Arg<Ts>...>::template get<Arg<T>>();
    }
    template<typename T>
    const Arg<T>& get() const {
        return TypesChain<Arg<Ts>...>::template get<Arg<T>>();
    }
};

template<template<typename >typename Arg, typename ...Ts>
auto make_subtypes_chain(Arg<Ts>&&... args) {
    return SubTypesChain<Arg, Ts...>(std::move(args)...);
}
