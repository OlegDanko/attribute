#pragma once

#include "GameState_decl.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ...RW, typename ...RO>
struct GS_impl<H, A>::GameStateFrame<types<RW...>, types<RO...>> {
    using write_buffers_tpl_t = typename type_apply<WriteBuffer, RW...>::types_::tpl;
    using read_buffers_tpl_t = typename type_apply<ReadBuffer, RO...>::types_::tpl;
    write_buffers_tpl_t write_buffers;
    read_buffers_tpl_t read_buffers;
    GameStateFrame(write_buffers_tpl_t write, read_buffers_tpl_t read)
        : write_buffers(std::move(write))
        , read_buffers(std::move(read))
    {}

    template<typename T>
    T* gen(size_t id) {
        constexpr auto index = index_of_v<T, RW...>;
        auto& buf = std::get<index>(write_buffers);
        return buf->template gen<T>(id);
    }

    template<typename T, typename ...Ts>
    void gen_mutly(size_t id) {
        gen<T>(id);
        if constexpr (types<Ts...>::count > 0)
            gen_mutly<Ts...>(id);
    }

    template<typename T>
    T* get_writable(size_t id) {
        auto& buf = std::get<index_of_v<T, RW...>>(write_buffers);
        return buf->template get<T>(id);
    }
    template<typename T>
    const T* get_readable(size_t id) {
        auto& buf = std::get<index_of_v<T, RO...>>(read_buffers);
        return buf->template read<T>(id);
    }
};
