#pragma once

#include "GameState_decl.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ...QTs>
struct GS_impl<H, A>::clients_collector<types<>, types<QTs...>> {
    using queue_tpl_t = typename type_apply<AttrStateQueue, QTs...>::types_::tpl;

    static constexpr std::tuple<> get(queue_tpl_t& queue_tpl) {
        return {};
    };
};

template<template<typename> typename H, template<typename> typename A>
template<typename CT, typename ...CTs, typename ...QTs>
struct GS_impl<H, A>::clients_collector<types<CT, CTs...>, types<QTs...>> {
    using clients_tpl_t = typename type_apply<AttrClient, CT, CTs...>::types_::tpl;
    using queue_tpl_t = typename type_apply<AttrStateQueue, QTs...>::types_::tpl;

    using sub_collector_t = clients_collector<types<CTs...>, types<QTs...>>;

    static constexpr clients_tpl_t get(queue_tpl_t& queue_tpl) {
        return std::tuple_cat(
            std::tuple(std::get<index_of<CT, QTs...>::i>(queue_tpl).create_client()),
            sub_collector_t::get(queue_tpl));
    };
};
