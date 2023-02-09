#pragma once

#include "EventListener.hpp"
#include "ITriggerable.hpp"

#include <unordered_set>
#include <unordered_map>

template<typename ...Args>
struct EventDispatcher : ITriggerable {
    using event_t = Event<Args...>;
    using event_s_ptr_t = std::shared_ptr<event_t>;
    using listener_t = IEventListener<Args...>;
    using listeners_t = std::unordered_set<listener_t*>;
    using id_event_s_ptr_pair_t = std::pair<size_t, event_s_ptr_t>;

    listeners_t listeners;
    std::unordered_map<size_t, listeners_t> listeners_direct;

    std::queue<event_s_ptr_t> staged;
    std::queue<id_event_s_ptr_pair_t> staged_direct;

    void reg_listener(listener_t* l) {
        listeners.insert(l);
    }

    void stage(event_s_ptr_t e) {
        staged.emplace(std::move(e));
    }

    void fire(event_s_ptr_t e) {
        for(auto l : listeners) {
            l->notify(e);
        }
    }

    void trigger() override {
        while(!staged.empty()) {
            for(auto l : listeners) {
                l->notify(staged.front());
            }
            staged.pop();
        }
    }
};
