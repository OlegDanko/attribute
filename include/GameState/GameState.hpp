#pragma once

#include "GameState_decl.hpp"
#include "GameStateClient.hpp"
#include "GameStateClientsCollector.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ...Ts>
struct GS_impl<H, A>::GameState<types<Ts...>> {
    using state_queue_tpl_t = typename type_apply<AttrStateQueue, Ts...>::types_::tpl;
    state_queue_tpl_t state_queues_tpl;

    template<typename RW, typename RO>
    GameStateClient<RW, RO> get_client() {
        return GameStateClient<RW, RO>(
                    clients_collector<RW, types<Ts...>>::get(state_queues_tpl),
                    clients_collector<RO, types<Ts...>>::get(state_queues_tpl)
                    );
    }

    auto get_client() {
        return get_client<types<Ts...>, types<>>();
    }

    template<typename CLIENT>
    auto get_client() {
        return get_client<typename CLIENT::RW_t, typename CLIENT::RO_t>();
    }
};
