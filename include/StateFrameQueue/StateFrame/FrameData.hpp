#pragma once

#include "FrameDataState.hpp"

template<typename T>
class FrameData {
    FrameDataState<T> state;
    FrameDataUpdate<T> update;
public:
    FrameData() = default;
    FrameData(const FrameDataState<T>& state, FrameDataUpdate<T> upd)
        : state(state, upd)
        , update(std::move(upd)) {}
    const FrameDataState<T>& get_state() { return state; }
    void merge_from(FrameData& from) {
        update.merge_from(from.update);
    }
    void update_from(const FrameData& from) {
        update.update_from(from.update);
    }
};
