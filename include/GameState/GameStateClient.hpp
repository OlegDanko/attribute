#pragma once

#include "GameState_decl.hpp"
#include "GameStateFrame.hpp"
#include "GameStateFrameCollector.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ...RW, typename ...RO>
struct GS_impl<H, A>::GameStateClient<types<RW...>, types<RO...>> {
    using RW_t = types<RW...>;
    using RO_t = types<RO...>;
    using rw_clients_tpl_t = typename type_apply<AttrClient, RW...>::types_::tpl;
    using r_clients_tpl_t = typename type_apply<AttrClient, RO...>::types_::tpl;
    rw_clients_tpl_t read_write_clients;
    r_clients_tpl_t read_only_clients;

    GameStateClient(rw_clients_tpl_t rw_clients, r_clients_tpl_t r_clients)
        : read_write_clients(rw_clients)
        , read_only_clients(r_clients) {}

    using frame_t = GameStateFrame<types<RW...>, types<RO...>>;

    frame_t get_frame() {
        return frame_t (
            game_frame_collector<types<RW...>>::get_write_buffers(read_write_clients),
            std::tuple_cat(
                game_frame_collector<types<RW...>>::get_read_buffers(read_write_clients),
                game_frame_collector<types<RO...>>::get_read_buffers(read_only_clients)
             )
        );
    }
};
