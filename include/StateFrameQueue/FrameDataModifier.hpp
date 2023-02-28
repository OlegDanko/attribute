#pragma once

#include "StateFrame/FrameDataState.hpp"

enum ReadType {
    NEWEST,
    OLDEST
};

template<typename T>
class FrameDataModifier {
    FrameDataUpdate<T> update;
    const FrameDataState<T>& state;
public:
    FrameDataModifier(const FrameDataState<T>& state) : state(state) {}
    T* get(size_t id) {
        if(auto ptr = update.get(id); ptr)
            return ptr;
        if(auto ptr = state.read(id); ptr)
            return update.gen(id, *ptr);
        return nullptr;
    }
    template<ReadType rt = OLDEST>
    const T* read(size_t id) const {
        if constexpr(rt == NEWEST) {
            if(auto ptr = update.get(id); ptr)
                return ptr;
        }
        return state.read(id);
    }

    auto const_iter_range() const {
        return state.const_iter_range();
    }

    FrameDataUpdate<T> take_updates() { return std::move(update); }
};
