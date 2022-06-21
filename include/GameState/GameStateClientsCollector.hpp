#pragma once

#include "GameState_decl.hpp"

template<typename ...QTs>
struct clients_collector<types<>, types<QTs...>> {
    using queue_t = typename type_apply<StateFrameQueue, QTs...>::types_::tpl;
    static auto collect_gen(queue_t& q) {
        return std::tuple<>();
    }
    static auto collect_mod(queue_t& q) {
        return std::tuple<>();
    }
    static auto collect_read(queue_t& q) {
        return std::tuple<>();
    }
};

template<typename T, typename ...QTs>
struct clients_collector<types<T>, types<QTs...>> {
    using queue_t = typename type_apply<StateFrameQueue, QTs...>::types_::tpl;
    static auto collect_gen(queue_t& q) {
        return std::make_tuple(std::get<index_of_v<T, QTs...>>(q).get_gen_provider());
    }
    static auto collect_mod(queue_t& q) {
        return std::make_tuple(std::get<index_of_v<T, QTs...>>(q).get_mod_provider());
    }
    static auto collect_read(queue_t& q) {
        return std::make_tuple(std::get<index_of_v<T, QTs...>>(q).get_read_provider());
    }
};

template<typename T, typename ...Ts, typename ...QTs>
struct clients_collector<types<T, Ts...>, types<QTs...>> {
    using queue_t = typename type_apply<StateFrameQueue, QTs...>::types_::tpl;
    using head_t = types<T>;
    using tail_t = types<Ts...>;
    using q_t = types<QTs...>;

    static auto collect_gen(queue_t& q) {
        return std::tuple_cat(clients_collector<head_t, q_t>::collect_gen(q),
                              clients_collector<tail_t, q_t>::collect_gen(q));
    }
    static auto collect_mod(queue_t& q) {
        return std::tuple_cat(clients_collector<head_t, q_t>::collect_mod(q),
                              clients_collector<tail_t, q_t>::collect_mod(q));
    }
    static auto collect_read(queue_t& q) {
        return std::tuple_cat(clients_collector<head_t, q_t>::collect_read(q),
                              clients_collector<tail_t, q_t>::collect_read(q));
    }
};
