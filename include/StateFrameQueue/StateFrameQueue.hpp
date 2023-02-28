#pragma once

#include "StateFrameQueueDecl.hpp"
#include "GenFrameProvider.hpp"
#include "ModFrameProvider.hpp"
#include "ReadFrameProvider.hpp"

template<typename T>
StateFrameQueue<T>::StateFrameQueue() : top_frame(std::make_unique<StateFrame<T>>()) {}

template<typename T>
const FrameDataState<T>& StateFrameQueue<T>::get_state() {
    std::lock_guard lk(mtx);
    return top_frame->get_state();
}

template<typename T>
const IFrameDataReader<T>& StateFrameQueue<T>::observe(size_t id) {
    std::lock_guard lk(mtx);
    observed_map[id] = top_frame.get();
    top_frame->observe(id);
    return top_frame->get_state();
}

template<typename T>
void StateFrameQueue<T>::unobserve(size_t id) {
    std::lock_guard lk(mtx);
    IF_PRESENT(id, observed_map, it) {
        it->second->unobserve(id);
        observed_map.erase(it);
        return;
    }
    throw std::logic_error("No frame observed by id " + std::to_string(id) + " found");
}

template<typename T>
void StateFrameQueue<T>::apply_update(FrameDataUpdate<T> upd, bool gen) {
    std::lock_guard lk(mtx);
    top_frame = std::make_unique<StateFrame<T>>(std::move(top_frame),
                                                std::move(upd),
                                                gen);
}

template<typename T>
GenFrameProvider<T> StateFrameQueue<T>::get_gen_provider() {  return {*this}; }

template<typename T>
ModFrameProvider<T> StateFrameQueue<T>::get_mod_provider() {  return {*this}; }

template<typename T>
ReadFrameProvider<T> StateFrameQueue<T>::get_read_provider() {
    static std::atomic_size_t ids = 0;
    return {ids.fetch_add(1), *this};
}
