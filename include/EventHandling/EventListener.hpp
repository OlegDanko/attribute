#pragma once

#include "Event.hpp"
#include "IListener.hpp"

#include <utils/utils.hpp>

#include <queue>
#include <mutex>

template<typename ...Args>
struct EventListener : IEventListener{
    using event_t = Event<Args...>;
    std::queue<std::shared_ptr<event_t>> events;
    std::mutex mtx;

    void notify(std::shared_ptr<event_t> e) {
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
