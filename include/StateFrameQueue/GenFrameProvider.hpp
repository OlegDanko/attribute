#pragma once
#include "StateFrameQueueDecl.hpp"

template<typename T>
class GenFrameDataHolder {
    StateFrameQueue<T>& upd_receiver;
    FrameDataUpdate<T> updates;
public:
    GenFrameDataHolder(GenFrameDataHolder&&) = delete;
    GenFrameDataHolder(const GenFrameDataHolder&) = delete;
    GenFrameDataHolder(StateFrameQueue<T>& receiver) : upd_receiver(receiver) {}
    ~GenFrameDataHolder() { upd_receiver.apply_update(std::move(updates), true); }
    IFrameDataUpdater<T>* operator->() { return &updates; }
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
