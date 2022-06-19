#pragma once

#include "StateFrame.hpp"
#include <mutex>


template<typename T>
class StateFrameQueue{
    std::unique_ptr<StateFrame<T>> top_frame;
    std::unordered_map<size_t, StateFrame<T>*> observed_map;
    std::mutex mtx;

public:

    const FrameDataState<T>& get_state() {
        std::lock_guard lk(mtx);
        return top_frame->get_state();
    }

    StateFrameQueue() : top_frame(std::make_unique<StateFrame<T>>()) {}

    const FrameDataReder<T>& observe(size_t id) {
        std::lock_guard lk(mtx);
        observed_map[id] = top_frame.get();
        top_frame->observe(id);
        return top_frame->get_state();
    }
    void unobserve(size_t id) {
        std::lock_guard lk(mtx);
        IF_PRESENT(id, observed_map, it) {
            it->second->unobserve(id);
            observed_map.erase(it);
            return;
        }
        throw std::logic_error("No frame observed by id " + std::to_string(id) + " found");
    }

    class ReadFrameProvider {
        size_t id;
        StateFrameQueue& observatory;

        struct FrameDataHolder {
            size_t id;
            StateFrameQueue& observatory;
            const FrameDataReder<T>& data;
            FrameDataHolder(size_t id,
                            StateFrameQueue& observatory)
                : id(id)
                , observatory(observatory)
                , data(observatory.observe(id)) {}
            ~FrameDataHolder() {
                observatory.unobserve(id);
            }

            const FrameDataReder<T>* operator->() { return &data; }
        };
    public:
        ReadFrameProvider(size_t id,
                          StateFrameQueue& observatory)
            : id(id)
            , observatory(observatory) {}
        FrameDataHolder get() { return { id, observatory }; }
    };
    ReadFrameProvider get_read_provider() {  static size_t ids = 0; return {++ids, *this}; }


    void set_update(FrameDataUpdate<T> upd, bool gen) {
        std::lock_guard lk(mtx);
        top_frame = std::make_unique<StateFrame<T>>(std::move(top_frame), std::move(upd), gen);
    }

    class GenFrameProvider {
        StateFrameQueue& upd_receiver;
    public:
        class FrameDataHolder {
            StateFrameQueue& upd_receiver;
            FrameDataUpdate<T> updates;
        public:
            FrameDataHolder(StateFrameQueue& receiver) : upd_receiver(receiver) {}
            ~FrameDataHolder() { upd_receiver.set_update(std::move(updates), true); }
            FrameDataUpdate<T>* operator->() { return &updates; }
        };
        GenFrameProvider(StateFrameQueue& receiver) : upd_receiver(receiver) {}
        FrameDataHolder get() { return {upd_receiver}; }
    };

    GenFrameProvider get_gen_provider() {  return {*this}; }

    class ModFrameProvider {
        StateFrameQueue& queue;
    public:
        class FrameDataHolder {
            StateFrameQueue<T>& queue;
            FrameDataModifier<T> updates;
        public:
            FrameDataHolder(StateFrameQueue<T>& queue)
                : queue(queue)
                , updates(queue.get_state())
            {}
            ~FrameDataHolder() { queue.set_update(std::move(updates.take_updates()), false); }
            FrameDataModifier<T>* operator->() { return &updates; }
        };
        ModFrameProvider(StateFrameQueue<T>& queue) : queue(queue) {}
        FrameDataHolder get() { return {queue}; }
    };

    ModFrameProvider get_mod_provider() {  return {*this}; }
};
