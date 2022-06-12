#pragma once

#include "GameState_decl.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ... Ts, size_t I>
struct GS_impl<H, A>::game_frame_collector<types<Ts...>, I> {
    using clients_tpl_t = typename type_apply<AttrClient, Ts...>::types_::tpl;
    using tail_types = typename types_tail<types<Ts...>, I>::types_;
    using write_buffers_tpl_t = typename type_apply<WriteBuffer, tail_types>::types_::tpl;
    using read_buffers_tpl_t = typename type_apply<ReadBuffer, tail_types>::types_::tpl;

    static write_buffers_tpl_t get_write_buffers(clients_tpl_t& clients) {
        if constexpr (types<Ts...>::count == 0)
            return std::make_tuple();
        else if constexpr (types<Ts...>::count == I+1)
            return std::make_tuple(std::get<I>(clients).get_write_buffer());
        else
            return std::tuple_cat(std::make_tuple(std::get<I>(clients).get_write_buffer()),
                                  game_frame_collector<types<Ts...>, I+1>::get_write_buffers(clients));
    }

    static read_buffers_tpl_t get_read_buffers(clients_tpl_t& clients) {
        if constexpr (types<Ts...>::count == 0)
            return std::make_tuple();
        else if constexpr (types<Ts...>::count == I+1)
            return std::make_tuple(std::get<I>(clients).get_read_buffer());
        else
            return std::tuple_cat(std::make_tuple(std::get<I>(clients).get_read_buffer()),
                                  game_frame_collector<types<Ts...>, I+1>::get_read_buffers(clients));
    }
};

