#pragma once

#include "GameState_decl.hpp"
#include "GameStateClient.hpp"
#include "GameStateClientsCollector.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ...Ts>
struct GS_impl<H, A>::GameState {
    using state_queue_tpl_t = typename type_apply<AttrStateQueue, Ts...>::types_::tpl;
    state_queue_tpl_t state_queues_tpl;

    template<typename RW = types<Ts...>, typename R = types<>>
    GameStateClient<RW, R> get_client() {
        return GameStateClient<RW, R>{
            clients_collector<RW, types<Ts...>>::get(state_queues_tpl),
            clients_collector<R, types<Ts...>>::get(state_queues_tpl)
        };
    }
};
