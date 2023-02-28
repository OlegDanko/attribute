#pragma once

#include <memory>
#include <unordered_set>
#include <utils/utils.hpp>

template<typename T>
class FrameDataState;

template<typename T>
class IFrameDataUpdater {
public:
    virtual T* gen(size_t) = 0;
    virtual T* get(size_t) = 0;
    virtual void remove(size_t) = 0;
};

template<typename T>
class FrameDataUpdate : public IFrameDataUpdater<T> {
    friend class FrameDataState<T>;
    std::unordered_map<size_t, std::unique_ptr<T>> u_ptr_map;
    std::unordered_set<size_t> remove_set;
public:
    FrameDataUpdate() = default;
    FrameDataUpdate(FrameDataUpdate&&) = default;
    FrameDataUpdate(const FrameDataUpdate&) = delete;

    void merge_from(FrameDataUpdate& from) {
        for(auto r = remove_set.begin(); r != remove_set.end(); ) {
            IF_PRESENT(*r, from.u_ptr_map, it) {
                from.u_ptr_map.erase(it);
                r = remove_set.erase(r);
            } else {
                remove_set.insert(*r);
                ++r;
            }
        }

        for(auto& [id, u_ptr] : u_ptr_map)
            from.u_ptr_map[id] = std::move(u_ptr);
        u_ptr_map = std::move(from.u_ptr_map);
    }

    void update_from(const FrameDataUpdate& from) {
        for(auto r = from.remove_set.begin(); r != from.remove_set.end(); ) {
            IF_PRESENT(*r, u_ptr_map, it) {
                u_ptr_map.erase(it);
            }
        }
    }

    T* gen(size_t id, const T& ref) {
        auto u_ptr = std::make_unique<T>(ref);
        auto ptr = u_ptr.get();
        u_ptr_map[id] = std::move(u_ptr);
        return ptr;
    }
    // FrameDataUpdater
    T* gen(size_t id) override {
        auto u_ptr = std::make_unique<T>(id);
        auto ptr = u_ptr.get();
        u_ptr_map[id] = std::move(u_ptr);
        return ptr;
    }

    T* get(size_t id) override {
        IF_PRESENT(id, u_ptr_map, it)
                return it->second.get();
        return nullptr;
    }

    void remove(size_t id) override {
        IF_PRESENT(id, u_ptr_map, it)
                u_ptr_map.erase(it);
        remove_set.insert(id);
    }
};
