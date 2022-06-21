#pragma once

#include "GameState_decl.hpp"
#include "GameStateClientsCollector.hpp"

template<typename ...Ts>
struct GameState {
    using queue_t = typename type_apply<StateFrameQueue, Ts...>::types_::tpl;
    queue_t queues;

    GameState(types<Ts...> = types<Ts...>()) {}

    using all_types = types<Ts...>;

    template<typename Types = all_types>
    auto get_gen_clients() {
        return clients_collector<Types, types<Ts...>>::collect_gen(queues);
    }
    template<typename Types = all_types>
    auto get_mod_clients() {
        return clients_collector<Types, types<Ts...>>::collect_mod(queues);
    }
    template<typename Types = all_types>
    auto get_read_clients() {
        return clients_collector<Types, types<Ts...>>::collect_read(queues);
    }
};
