#pragma once

#include "EventListener.hpp"
#include "ITriggerable.hpp"

template<typename ...Args>
struct EventDispatcher : ITriggerable {
    using event_t = Event<Args...>;
    using event_s_ptr_t = std::shared_ptr<event_t>;
    using listener_t = EventListener<Args...>;
    std::vector<listener_t*> listeners;
    std::queue<event_s_ptr_t> staged;

    void reg_listener(listener_t* l) {
        listeners.push_back(l);
    }

    void stage(event_s_ptr_t e) {
        staged.emplace(std::move(e));
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
