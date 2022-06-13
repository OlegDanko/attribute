#pragma once

#include "IListener.hpp"

#include <utils/utils.hpp>

#include <queue>
#include <mutex>

template<typename ...Args>
struct EventListenerQueue : IEventListener<Args...>, IEventServer {
    using event_t = Event<Args...>;
    std::queue<std::shared_ptr<event_t>> events;
    std::mutex mtx;

    void notify(std::shared_ptr<event_t> e) override {
        with(std::lock_guard lk(mtx)) {
            events.push(std::move(e));
        }
    }

    virtual void serve_event(event_t& e) = 0;

    void serve_events() override {
        std::queue<std::shared_ptr<event_t>> evs;
        with(std::lock_guard lk(mtx)) {
            evs = std::move(events);
            events = {};
        }
        while(!evs.empty()) {
            serve_event(*evs.front());
            evs.pop();
        }
    }
};

struct IEventListenerHolder {};

template<typename ...Args>
struct EventListenerInstant : IEventListener<Args...>, IEventListenerHolder {
    using event_t = Event<Args...>;
    std::mutex mtx;

    void notify(std::shared_ptr<event_t> e) override {
        with(std::lock_guard lk(mtx)) {
            serve_event(*e);
        }
    }

    virtual void serve_event(event_t& e) = 0;
};

