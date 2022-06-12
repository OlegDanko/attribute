#pragma once

#include "EventListener.hpp"
#include "EventDispatcher.hpp"

template<typename ...Args>
struct AttributeEventListener : EventListener<Args...> {
    using fn_t = std::function<void(size_t, Args...)>;
    fn_t callback_fn;

    void serve_event(typename EventListener<Args...>::event_t& e) override {
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
};


template<typename T>
struct TypeByIdProvider {
    virtual T& get(size_t id) = 0;
};

template<typename T, typename ...Args>
std::unique_ptr<IEventListener> bind_event_listener(EventDispatcher<Args...>& src, TypeByIdProvider<T>& p, void(T::*fn)(Args...)) {
    auto l = std::make_unique<AttributeEventListener<Args...>>();
    l->callback_fn = [&p, fn](size_t id, Args... args){
        (p.get(id).*fn)(args...);
    };
    src.reg_listener(l.get());
    return std::move(l);
}
