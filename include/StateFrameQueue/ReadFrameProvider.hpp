#pragma once
#include "StateFrameQueueDecl.hpp"
#include <atomic>

template<typename T>
struct ReadFrameDataHolder {
    size_t id;
    StateFrameQueue<T>& observatory;
    const IFrameDataReader<T>& data;
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

    const IFrameDataReader<T>* operator->() { return &data; }
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
