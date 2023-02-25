#pragma once
#include <cstddef>
#include <vector>

//This is to distinguish between IObjectGenListener of different types
template<typename T>
struct IdTypeWrapper {
    size_t id;
    IdTypeWrapper(size_t id) : id{id} {}
    operator size_t() { return id; }
};

template<typename T>
class IObjectGenListener {
public:
    virtual void on_game_object_created(IdTypeWrapper<T> id) = 0;
//    virtual void on_game_object_removed(IdTypeWrapper<T> id) = 0;
};

template<typename ...Ts>
struct ObjectGenListenerRegister;

template<typename T>
struct ObjectGenListenerRegister<T> {
    std::vector<IObjectGenListener<T>*> listeners;

    template<typename A>
    void add(IObjectGenListener<A>* listener) {
        if constexpr(std::is_same<A, T>::value) {
            listeners.push_back(listener);
        }
    }

    template<typename ATTR>
    void on_generated_(size_t id) {
        if constexpr(std::is_same<ATTR, T>::value) {
            for(auto listener : listeners) {
                listener->on_game_object_created(id);
            }
        }
    }
    template<typename ATTR>
    void on_generated(size_t id) {
        on_generated_<ATTR>(id);
    }
};

template<typename T, typename ...Ts>
struct ObjectGenListenerRegister<T, Ts...> {
    ObjectGenListenerRegister<Ts...> regs;
    std::vector<IObjectGenListener<T>*> listeners;

    template<typename ATTR>
    void add(IObjectGenListener<ATTR>* listener) {
        if constexpr(std::is_same<ATTR, T>::value) {
            listeners.push_back(listener);
        } else {
            regs.add(listener);
        }
    }

    template<typename ATTR>
    void on_generated_(size_t id) {
        if constexpr(std::is_same<ATTR, T>::value) {
            for(auto listener : listeners) {
                listener->on_game_object_created(id);
            }
        } else {
            regs.template on_generated_<ATTR>(id);
        }
    }

    template<typename ...ATTRS>
    void on_generated(size_t id) {
        (on_generated_<ATTRS>(id), ...);
    }
};
