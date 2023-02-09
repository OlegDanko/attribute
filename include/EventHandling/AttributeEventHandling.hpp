#pragma once

#include "EventDispatcher.hpp"

template<template<typename ...>typename EL_base, typename ...Args>
struct AttributeEventListener : EL_base<Args...> {
    using fn_t = std::function<void(size_t, Args...)>;
    fn_t callback_fn;

    void serve_event(typename EL_base<Args...>::event_t& e) override {
        std::apply(callback_fn, std::tuple_cat(std::make_tuple(e.id), e.args));
    }
};

template <typename... Args>
struct AttributeEventDispatcher : EventDispatcher<Args...> {
    using base_t = EventDispatcher<Args...>;
    using event_t = typename base_t::event_t;
    void stage(size_t id, Args&...args) {
        base_t::stage(std::make_shared<event_t>(id, std::make_tuple(args...)));
    }
    void fire(size_t id, const Args&...args) {
        base_t::fire(std::make_shared<event_t>(id, std::make_tuple(args...)));
    }
};

template<typename T>
struct ITypeByIdProvider {
    virtual T* get(size_t id) = 0;
};

template<template<typename ...>typename EL, typename T_Provider, typename T, typename ...Args>
std::unique_ptr<EL<Args...>> bind_event_listener(EventDispatcher<Args...>& src, T_Provider& p, void(T::*fn)(Args...)) {
    auto l = std::make_unique<AttributeEventListener<EL, Args...>>();
    l->callback_fn = [&p, fn](size_t id, Args... args){
        auto ptr_ = p.get(id);
        T* ptr = ptr_;
        if(T* ptr = ptr_; ptr)
            (ptr->*fn)(args...);
    };
    src.reg_listener(l.get());
    return std::move(l);
}
