#pragma once
#include "FrameData.hpp"

template<typename T>
class StateFrame {
    std::unique_ptr<StateFrame> prev{nullptr};
    StateFrame* next{nullptr};

    FrameData<T> data;
    bool is_gen{false};

    std::unordered_set<size_t> observers;

    bool is_ready_to_merge() {
        return !is_gen && observers.empty();
    }

    void try_merge_prev() {
        if(!is_ready_to_merge() || !prev || !prev->is_ready_to_merge())
            return;

        data.merge_from(prev->data);

        if(!prev->prev) {
            prev = nullptr;
            return;
        }

        prev = std::move(prev->prev);
        prev->next = this;
    }

    void on_gen_applied() {
        is_gen = false;
        try_merge_prev();
    }

    // add new items and remove expired ones
    void apply_gen_frame(StateFrame& frame) {
        if(frame.prev && frame.prev->is_gen) {
            apply_gen_frame(*frame.prev);
        }
        data.update_from(frame.data);
        frame.on_gen_applied();
    }
public:
    const FrameDataState<T>& get_state() { return data.get_state(); }

    StateFrame() = default;
    StateFrame(StateFrame&&) = delete;
    StateFrame(const StateFrame&) = delete;
    StateFrame(std::unique_ptr<StateFrame> frame, FrameDataUpdate<T> upd, bool gen)
        : prev(std::move(frame))
        , data(prev->get_state(), std::move(upd))
        , is_gen(gen) {

        prev->next = this;

        if(is_gen)
            return;

        if(prev->is_gen) {
            apply_gen_frame(*prev);
        }
        try_merge_prev();
    }

    const FrameData<T>& observe(size_t id) {
        observers.insert(id);
        return data;
    }

    void unobserve(size_t id) {
        IF_PRESENT(id, observers, it)
            observers.erase(it);

        if(!observers.empty())
            return;

        if(next)
           next->try_merge_prev();
    }
};
