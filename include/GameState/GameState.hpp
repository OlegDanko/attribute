#pragma once

#include <utils/types/TypesChain.hpp>
#include <StateFrameQueue/StateFrameQueue.hpp>

using utl_prf::types;
using utl_prf::SubTypesChain;

template<typename ...Ts>
using queues_t = SubTypesChain<StateFrameQueue, Ts...>;

template<template<typename> typename PROVIDER>
struct provider_helper {
    static constexpr bool is_gen = std::is_same_v<PROVIDER<void>, GenFrameProvider<void>>;
    static constexpr bool is_mod = std::is_same_v<PROVIDER<void>, ModFrameProvider<void>>;
    static constexpr bool is_read = std::is_same_v<PROVIDER<void>, ReadFrameProvider<void>>;
};
template<typename T>
using GenFrameDataHolder_ptr_t = std::unique_ptr<GenFrameDataHolder<T>>;
template<typename T>
using ModFrameDataHolder_ptr_t = std::unique_ptr<ModFrameDataHolder<T>>;
template<typename T>
using ReadFrameDataHolder_ptr_t = std::unique_ptr<ReadFrameDataHolder<T>>;

template<template<typename> typename PVD, typename ...Ts>
struct FrameProviders : SubTypesChain<PVD, Ts...> {
    template<typename ...Qs>
    FrameProviders(queues_t<Qs...>& q)
        : base_t(gen_base(q)) {}

    auto get_frames() {
        if constexpr (pvd_helper::is_gen) {
            return utl_prf::make_subtypes_chain<GenFrameDataHolder_ptr_t>(
                        this->template get<Ts>().get()...);
        } else if constexpr (pvd_helper::is_mod) {
            return utl_prf::make_subtypes_chain<ModFrameDataHolder_ptr_t>(
                        this->template get<Ts>().get()...);
        } else if constexpr (pvd_helper::is_read) {
            return utl_prf::make_subtypes_chain<ReadFrameDataHolder_ptr_t>(
                        this->template get<Ts>().get()...);
        }
    }
private:
    using pvd_helper = provider_helper<PVD>;
    using base_t = SubTypesChain<PVD, Ts...>;

    template<typename ...Qs>
    auto gen_base(queues_t<Qs...>& q) {
        return utl_prf::make_subtypes_chain<PVD>(get_provider<Ts>(q)...);
    }

    template<typename T, typename ...Qs>
    auto get_provider(queues_t<Qs...>& q) {
        if constexpr (pvd_helper::is_gen) {
            return q.template get<T>().get_gen_provider();
        } else if constexpr (pvd_helper::is_mod) {
            return q.template get<T>().get_mod_provider();
        } else if constexpr (pvd_helper::is_read) {
            return q.template get<T>().get_read_provider();
        }
    }
};


template<typename T>
struct GameState;

template<typename ...Ts>
struct GameState<types<Ts...>> {
    using queue_t = SubTypesChain<StateFrameQueue, Ts...>;
    queue_t queues;

    GameState(types<Ts...> = types<Ts...>()) {}

    using all_types = types<Ts...>;

    template<typename ...Cs>
    auto get_gen_providers() {
        return FrameProviders<GenFrameProvider, Cs...>(queues);
    }
    auto get_gen_providers() {
        return get_gen_providers<Ts...>();
    }

    template<typename ...Cs>
    auto get_mod_providers() {
        return FrameProviders<ModFrameProvider, Cs...>(queues);
    }
    auto get_mod_providers() {
        return get_gen_providers<Ts...>();
    }

    template<typename ...Cs>
    auto get_read_providers() {
        return FrameProviders<ReadFrameProvider, Cs...>(queues);
    }
    auto get_read_providers() {
        return get_gen_providers<Ts...>();
    }
};
