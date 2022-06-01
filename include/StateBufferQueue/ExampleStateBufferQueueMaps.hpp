#pragma once
#include <utils/utils.hpp>

//#include <memory>
//#include <unordered_map>
#include <unordered_set>


template<typename T>
class AccessorMap;

template<typename T>
class HolderMap {
    friend class AccessorMap<T>;

    std::unordered_map<size_t, std::unique_ptr<T>> map;
    std::unordered_set<size_t> removed;
public:
    void merge_from(HolderMap&& from) {
        for(auto id : from.removed) {
            IF_PRESENT(id, map, it)
                map.erase(id);
            else
                removed.insert(id);
        }

        for(auto& [id, obj] : from.map)
            map[id] = std::move(obj);
    }

    template<typename T_ = T>
    T_* gen(size_t id) {
        static_assert(std::is_base_of_v<T, T_>);
        IF_PRESENT(id, removed, it)
                removed.erase(it);
        auto u_ptr = std::make_unique<T_>();
        auto ptr = u_ptr.get();
        map[id] = std::move(u_ptr);
        return ptr;
    }

    template<typename T_ = T>
    T_* get(size_t id) {
        static_assert(std::is_base_of_v<T, T_>);
        IF_PRESENT(id, map, it)
                return it.second;
        return nullptr;
    }

    template<typename T_ = T>
    void remove(size_t id) {
        static_assert(std::is_base_of_v<T, T_>);
        IF_PRESENT(id, map, it) {
            map.erase(it);
        }
        removed.insert(id);
    }
};

template<typename T>
class AccessorMap {
    std::unordered_map<size_t, T*> map;
public:
    template<typename T_ = T>
    const T_* read(size_t id) const {
        static_assert(std::is_base_of_v<T, T_>);
        IF_PRESENT(id, map, it)
                return it->second;
        return nullptr;
    }
    void update(const HolderMap<T>& from) {
        for(auto id : from.removed)
            IF_PRESENT(id, map, it)
                map.erase(id);

        for(auto& [id, obj] : from.map)
            map[id] = obj.get();
    }
};
