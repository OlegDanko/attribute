#pragma once

#include "StateFrame.hpp"
#include <mutex>

template<typename T>
class StateFrameQueue;

template<typename T>
struct ReadFrameDataHolder {
    size_t id;
    StateFrameQueue<T>& observatory;
    const FrameDataReder<T>& data;
    ReadFrameDataHolder(ReadFrameDataHolder&&) = delete;
    ReadFrameDataHolder(const ReadFrameDataHolder&) = delete;
    ReadFrameDataHolder(size_t id,
                    StateFrameQueue<T>& observatory)
        : id(id)
        , observatory(observatory)
        , data(observatory.observe(id)) {}
    ~ReadFrameDataHolder() {
        observatory.unobserve(id);
    }

    const FrameDataReder<T>* operator->() { return &data; }
};

template<typename T>
class ReadFrameProvider {
    using data_holder_t = ReadFrameDataHolder<T>;
    size_t id;
    StateFrameQueue<T>& observatory;
public:
    ReadFrameProvider(size_t id,
                      StateFrameQueue<T>& observatory)
        : id(id)
        , observatory(observatory) {}
    std::unique_ptr<data_holder_t> get() {
        return std::make_unique<data_holder_t>(id, observatory);
    }
};

template<typename T>
class GenFrameDataHolder {
    StateFrameQueue<T>& upd_receiver;
    FrameDataUpdate<T> updates;
public:
    GenFrameDataHolder(GenFrameDataHolder&&) = delete;
    GenFrameDataHolder(const GenFrameDataHolder&) = delete;
    GenFrameDataHolder(StateFrameQueue<T>& receiver) : upd_receiver(receiver) {}
    ~GenFrameDataHolder() { upd_receiver.set_update(std::move(updates), true); }
    FrameDataUpdater<T>* operator->() { return &updates; }
};

template<typename T>
class GenFrameProvider {
    using data_holder_t = GenFrameDataHolder<T>;
    StateFrameQueue<T>& upd_receiver;
public:
    GenFrameProvider(StateFrameQueue<T>& receiver) : upd_receiver(receiver) {}
    std::unique_ptr<data_holder_t> get() {
        return std::make_unique<data_holder_t>(upd_receiver);
    }
};

template<typename T>
class ModFrameDataHolder {
    StateFrameQueue<T>& queue;
    FrameDataModifier<T> updates;
public:
    ModFrameDataHolder(ModFrameDataHolder&&) = delete;
    ModFrameDataHolder(const ModFrameDataHolder&) = delete;
    ModFrameDataHolder(StateFrameQueue<T>& queue)
        : queue(queue)
        , updates(queue.get_state())
    {}
    ~ModFrameDataHolder() {
        queue.set_update(std::move(updates.take_updates()), false);
    }
    FrameDataModifier<T>* operator->() { return &updates; }
};

template<typename T>
class ModFrameProvider {
    using data_holder_t = ModFrameDataHolder<T>;
    StateFrameQueue<T>& queue;
public:
    ModFrameProvider(StateFrameQueue<T>& queue) : queue(queue) {}
    std::unique_ptr<data_holder_t> get() {
        return std::make_unique<data_holder_t>(queue);
    }
};

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

    void set_update(FrameDataUpdate<T> upd, bool gen) {
        std::lock_guard lk(mtx);
        top_frame = std::make_unique<StateFrame<T>>(std::move(top_frame),
                                                    std::move(upd),
                                                    gen);
    }

    ReadFrameProvider<T> get_read_provider() {  static size_t ids = 0; return {++ids, *this}; }
    GenFrameProvider<T> get_gen_provider() {  return {*this}; }
    ModFrameProvider<T> get_mod_provider() {  return {*this}; }
};
