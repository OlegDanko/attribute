#pragma once

#include "GameState_decl.hpp"

template<template<typename> typename H, template<typename> typename A>
template<typename ...RW, typename ...RO>
class GS_impl<H, A>::GameStateFrame<types<RW...>, types<RO...>> {
    using write_buffers_tpl_t = typename type_apply<WriteBuffer, RW...>::types_::tpl;
    using read_buffers_tpl_t = typename type_apply<ReadBuffer, RW..., RO...>::types_::tpl;
    write_buffers_tpl_t write_buffers;
    read_buffers_tpl_t read_buffers;

protected:
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

public:
    GameStateFrame(write_buffers_tpl_t write, read_buffers_tpl_t read)
        : write_buffers(std::move(write))
        , read_buffers(std::move(read))
    {}

    class GameObject {
        GameStateFrame& frame;
        size_t id;
    public:
        GameObject(GameStateFrame& frame, size_t id)
            : frame(frame)
            , id(id) {}

        template<typename T>
        T* get_attr() {
            if constexpr(!is_present_v<T, RW...>)
                return nullptr;
            else {
                constexpr auto index_w = index_of_v<T, RW...>;
                auto& buf_w = std::get<index_w>(frame.write_buffers);
                if(auto ptr = buf_w->template get<T>(id); ptr)
                    return ptr;

                constexpr auto index_r = index_of_v<T, RW..., RO...>;
                auto& buf_r = std::get<index_r>(frame.read_buffers);

                if(auto ptr = buf_r->template read<T>(id); ptr)
                    return buf_w->template gen(*ptr);
            }
            return nullptr;
        }
        template<typename T, bool prev_only = false>
        const T* read_attr() {
            if constexpr(!is_present_v<T, RW..., RO...>)
                return nullptr;
            else if constexpr (prev_only || !is_present_v<T, RW...>) {
                constexpr auto index_r = index_of_v<T, RW..., RO...>;
                auto& buf_r = std::get<index_r>(frame.read_buffers);

                return buf_r->template read<T>(id);
            } else {
                constexpr auto index_w = index_of_v<T, RW...>;
                auto& buf_w = std::get<index_w>(frame.write_buffers);
                if(auto ptr = buf_w->template get<T>(id); ptr)
                    return ptr;
            }
        }
    };

    GameObject get(size_t id) {
        return {*this, id};
    }

    template<typename IT>
    struct GO_Iter {
        GameStateFrame& frame;
        IT it;
        GO_Iter(GameStateFrame& frame, IT it) : frame(frame), it(it) {}
        bool operator!=(GO_Iter that) { return it != that.it; }
        GameObject operator*() { return GameObject(frame, it->first); }
        GO_Iter& operator++() { ++it; return *this ; }
    };

    template<typename IT>
    struct TypeRange {
        IT begin_;
        IT end_;
        IT begin() { return begin_; }
        IT end() { return end_; }
    };

    template<typename T>
    auto iter_go() {
        auto& buf = **std::get<index_of_v<T, RW..., RO...>>(read_buffers);
        using it_t = GO_Iter<decltype(buf.begin())>;
        return TypeRange<it_t>{.begin_ = it_t(*this, buf.begin()),
                               .end_ = it_t(*this, buf.end())};
    }
};
