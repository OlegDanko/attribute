#pragma once
#include <memory>
#include <vector>

template<typename ...Args>
struct Event {
    using args_t = std::tuple<Args...>;
    size_t id;
    args_t args;
    Event(size_t id, std::tuple<Args...> args)
        : id(id)
        , args(std::move(args)) {}
};

template<typename ...Args>
struct Event_cross {
    using args_t = std::tuple<Args...>;
    size_t id_src;
    using ids_dest_t = std::vector<size_t>;
    ids_dest_t ids_dest;
    args_t args;
    Event_cross(size_t id, ids_dest_t ids, std::tuple<Args...> args)
        : id_src(id)
        , ids_dest(std::move(ids))
        , args(std::move(args)) {}
};
