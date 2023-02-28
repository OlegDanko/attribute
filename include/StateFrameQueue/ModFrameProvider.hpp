#pragma once
#include "StateFrameQueueDecl.hpp"
#include "FrameDataModifier.hpp"

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
        queue.apply_update(std::move(updates.take_updates()), false);
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
