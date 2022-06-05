#pragma once
#include <memory>

template<typename ...Args>
struct Event {
    using args_t = std::tuple<Args...>;
    size_t id;
    args_t args;
    Event(size_t id, std::tuple<Args...> args)
        : id(id)
        , args(std::move(args)) {}
};
