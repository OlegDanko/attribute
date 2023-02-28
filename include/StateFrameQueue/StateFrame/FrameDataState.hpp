#pragma once

#include "FrameDataUpdate.hpp"

template<typename T>
class IFrameDataReader {
    using ptr_map_t = std::unordered_map<size_t, T*>;
    using ptr_map_citer_t = typename ptr_map_t::const_iterator;
public:
    virtual T* read(size_t) const = 0;
    virtual std::pair<ptr_map_citer_t, ptr_map_citer_t> const_iter_range() const = 0;
};

template<typename T>
class FrameDataState : public IFrameDataReader<T> {
    using ptr_map_t = std::unordered_map<size_t, T*>;
    using ptr_map_citer_t = typename ptr_map_t::const_iterator;
    ptr_map_t ptr_map;
public:
    FrameDataState() = default;
    FrameDataState(const FrameDataState& state, const FrameDataUpdate<T>& upd)
        : ptr_map(state.ptr_map) {
        for(auto r : upd.remove_set)
            IF_PRESENT(r, ptr_map, it)
                ptr_map.erase(it);

        for(auto& [id, u_ptr] : upd.u_ptr_map)
            ptr_map[id] = u_ptr.get();
    }
    void set(size_t id, T* ptr) {
        ptr_map[id] = ptr;
    }

    // FrameDataReder
    T* read(size_t id) const override {
        IF_PRESENT(id, ptr_map, it)
                return it->second;
        return nullptr;
    }
    std::pair<ptr_map_citer_t, ptr_map_citer_t> const_iter_range() const override {
        return {ptr_map.begin(), ptr_map.end()};
    }
};
