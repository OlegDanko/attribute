#pragma once

#include "StateFrame/StateFrame.hpp"
#include <mutex>

template<typename T> struct ReadFrameProvider;
template<typename T> struct GenFrameProvider;
template<typename T> struct ModFrameProvider;

template<typename T>
class StateFrameQueue{
    std::unique_ptr<StateFrame<T>> top_frame;
    std::unordered_map<size_t, StateFrame<T>*> observed_map;
    std::mutex mtx;
public:
    const FrameDataState<T>& get_state();

    StateFrameQueue();

    const IFrameDataReader<T>& observe(size_t id);
    void unobserve(size_t id);

    void apply_update(FrameDataUpdate<T> upd, bool gen);

    ReadFrameProvider<T> get_read_provider();
    GenFrameProvider<T> get_gen_provider();
    ModFrameProvider<T> get_mod_provider();
};
