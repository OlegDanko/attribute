#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utils/utils.hpp>

template<typename T>
class FrameDataState;

template<typename T>
class FrameDataUpdater {
public:
    virtual T* gen(size_t) = 0;
    virtual void remove(size_t) = 0;
};

template<typename T>
class FrameDataUpdate : FrameDataUpdater<T> {
    friend class FrameDataState<T>;
    std::unordered_map<size_t, std::unique_ptr<T>> u_ptr_map;
    std::unordered_set<size_t> remove_set;
public:
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

    T* get(size_t id) {
        IF_PRESENT(id, u_ptr_map, it)
                return it->second.get();
        return nullptr;
    }

    T* gen(size_t id, const T& ref) {
        auto u_ptr = std::make_unique<T>(ref);
        auto ptr = u_ptr.get();
        u_ptr_map[id] = std::move(u_ptr);
        return ptr;
    }
    // FrameDataUpdater
    T* gen(size_t id) override {
        auto u_ptr = std::make_unique<T>();
        auto ptr = u_ptr.get();
        u_ptr_map[id] = std::move(u_ptr);
        return ptr;
    }

    void remove(size_t id) override {
        IF_PRESENT(id, u_ptr_map, it)
                u_ptr_map.erase(it);
        remove_set.insert(id);
    }
};

template<typename T>
class FrameDataReder {
public:
    virtual T* read(size_t) const = 0;
    virtual void read_serve(std::function<void(size_t, const T&)>) const = 0;
};

template<typename T>
class FrameDataState : public FrameDataReder<T> {
    std::unordered_map<size_t, T*> ptr_map;
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
    void read_serve(std::function<void(size_t, const T&)> fn) const override {
        for(auto [id, ptr] : ptr_map) {
            fn(id, *ptr);
        }
    }
};


template<typename T>
class FrameDataModifier {
    FrameDataUpdate<T> update;
    const FrameDataState<T>& state;
public:
    FrameDataModifier(const FrameDataState<T>& state) : state(state) {}
    T* get(size_t id) {
        if(auto ptr = update.get(id); ptr)
            return ptr;
        if(auto ptr = state.read(id); ptr)
            return update.gen(id, *ptr);
        return nullptr;
    }
    FrameDataUpdate<T> take_updates() { return std::move(update); }
};

template<typename T>
class StateFrameData {
    FrameDataState<T> state;
    FrameDataUpdate<T> update;
public:
    StateFrameData() = default;
    StateFrameData(const FrameDataState<T>& state, FrameDataUpdate<T> upd)
        : state(state, upd)
        , update(std::move(upd)) {}
    const FrameDataState<T>& get_state() { return state; }
    void merge_from(StateFrameData& from) {
        update.merge_from(from.update);
    }
    void update_from(const StateFrameData& from) {
        update.update_from(from.update);
    }
};
