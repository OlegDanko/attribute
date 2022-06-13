#pragma once

#include "Event.hpp"

template<typename ...Args>
struct IEventListener {
    using event_t = Event<Args...>;
    virtual ~IEventListener() = default;
    virtual void notify(std::shared_ptr<event_t> e) = 0;
};

struct IEventServer {
    virtual ~IEventServer() = default;
    virtual void serve_events() = 0;
};
