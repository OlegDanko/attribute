#pragma once

#include "GameState_decl.hpp"
#include "GameState.hpp"

template<typename T, size_t I = 0>
auto collect_frames(T& clients) {
    if constexpr (std::tuple_size_v<T> == 0)
            return std::tuple<>();
    else if constexpr (I+1 == std::tuple_size_v<T>)
            return std::make_tuple(std::get<I>(clients).get());
    else
        return std::tuple_cat(std::make_tuple(std::get<I>(clients).get()),
                              collect_frames<T, I+1>(clients));
}

template<typename ...MOD, typename ... READ>
struct GameStateClient<types<MOD...>, types<READ...>> {
    using mod_clients_t = typename type_apply<ModFrameProvider, MOD...>::types_::tpl;
    using read_clients_t = typename type_apply<ReadFrameProvider, READ...>::types_::tpl;
    mod_clients_t mod_clients;
    read_clients_t read_clients;

    template<typename ...Ts>
    GameStateClient(GameState<Ts...>& gs)
        : mod_clients(gs.template get_mod_clients<types<MOD...>>())
        , read_clients(gs.template get_read_clients<types<READ...>>()) {}

    struct Frame {
        decltype(collect_frames(mod_clients)) mod_frames;
        decltype(collect_frames(read_clients)) read_frames;

        Frame(mod_clients_t& mod_clients, read_clients_t& read_clients)
            : mod_frames(std::move(collect_frames(mod_clients)))
            , read_frames(std::move(collect_frames(read_clients))) {}

        template<typename T>
        const T* read(size_t id) const {
            if constexpr (is_present_v<T, MOD...>) {
                return (*std::get<index_of_v<T, MOD...>>(mod_frames))->read(id);
            }
            else if constexpr (is_present_v<T, READ...>) {
                return (*std::get<index_of_v<T, READ...>>(read_frames))->read(id);
            }
            return nullptr;
        }

        template<typename T>
        T* get(size_t id) {
            if constexpr (is_present_v<T, MOD...>) {
                return (*std::get<index_of_v<T, MOD...>>(mod_frames))->get(id);
            }
            return nullptr;
        }

        struct GameObject {
            Frame& frame;
            size_t id;
            GameObject(Frame& frame, size_t id) : frame(frame), id(id) {}

            template<typename T>
            const T* read_attr() const {
                return frame.read<T>(id);
            }

            template<typename T>
            T* get_attr() {
                return frame.get<T>(id);
            }
        };

        GameObject get(size_t id) { return {*this, id}; }

        template<typename attr_it>
        class go_citer {
            Frame& frame;
            attr_it it;
        public:
            go_citer(Frame& f, attr_it i) : frame(f), it(i) {}
            auto operator*() { return frame.get(it->first); }
            void operator++() { it++; }
            bool operator==(const go_citer& that) const { return it == that.it; }
            bool operator!=(const go_citer& that) const { return !(*this == that); }
        };

        template<typename attr_it>
        std::pair<go_citer<attr_it>, go_citer<attr_it>>
        go_citers_from_attr_citers(attr_it begin, attr_it end) {
            return {
                go_citer<attr_it>{*this, begin},
                go_citer<attr_it>{*this, end}
            };
        }

        template<typename T>
        auto iterate_game_objects() {
            if constexpr (is_present_v<T, MOD...>) {
                auto [begin, end] = (*std::get<index_of_v<T, MOD...>>(mod_frames))
                        ->const_iter_range();
                auto [b, e] = go_citers_from_attr_citers(begin, end);
                return utl_prf::iterable(b, e);
            }
            if constexpr (is_present_v<T, READ...>) {
                auto [begin, end] = (*std::get<index_of_v<T, READ...>>(read_frames))
                        ->const_iter_range();
                auto [b, e] = go_citers_from_attr_citers(begin, end);
                return utl_prf::iterable(b, e);
            }
        }

    };

    auto get_frame() { return Frame(mod_clients, read_clients); }
};

template<typename>
struct GameStateGenClient;

struct game_object_id {
    static size_t id;
    static size_t get() { return ++id; }
};
size_t game_object_id::id{0};

class IObjectGenListener {
public:
    virtual void on_game_object_created(size_t id) = 0;
    virtual void on_game_object_removed(size_t id) = 0;
};

template<typename ...GEN>
struct GameStateGenClient<types<GEN...>> {
    using gen_clients_t = typename type_apply<GenFrameProvider, GEN...>::types_::tpl;
    gen_clients_t gen_clients;

    using gen_listeners_map_t = std::unordered_map<size_t, std::vector<IObjectGenListener*>>;
    gen_listeners_map_t gen_listeners;

    template<typename T>
    void add_listener(IObjectGenListener* l) {
        gen_listeners[typeid(T).hash_code()].push_back(l);
    }

    GameStateGenClient(GameState<types<GEN...>>& gs) : gen_clients(std::move(gs.get_gen_clients())) {}

    struct Frame {
        decltype(collect_frames(gen_clients)) gen_frames;
        gen_listeners_map_t& gen_listeners;

        Frame(gen_clients_t& gen_clients, gen_listeners_map_t& gen_listeners)
            : gen_frames(std::move(collect_frames(gen_clients)))
            , gen_listeners(gen_listeners) {}

        template<typename T>
        T* get(size_t id) {
            if constexpr (is_present_v<T, GEN...>) {
                return (*std::get<index_of_v<T, GEN...>>(gen_frames))->get(id);
            }
            return nullptr;
        }

        struct GameObject {
            Frame& frame;
            size_t id;
            GameObject(Frame& frame, size_t id) : frame(frame), id(id) {}

            auto get_id() { return id; }

            template<typename T>
            T* get_attr() {
                return frame.get<T>(id);
            }
        };

        template<typename...>
        struct generator;

        template<typename T>
        struct generator<T> {
            static void gen(size_t id, decltype(gen_frames)& frames, gen_listeners_map_t& ls) {
                (*std::get<index_of_v<T, GEN...>>(frames))->gen(id);
                for(auto l : ls[typeid(T).hash_code()]) l->on_game_object_created(id);
            }
        };

        template<typename T, typename ...Ts>
        struct generator<T, Ts...> {
            static void gen(size_t id, decltype(gen_frames)& frames, gen_listeners_map_t& ls) {
                generator<T>::gen(id, frames, ls);
                generator<Ts...>::gen(id, frames, ls);
            }
        };

        template<typename ...Ts>
        GameObject gen(types<Ts...> = types<Ts...>()) {
            auto id = game_object_id::get();
            generator<Ts...>::gen(id, gen_frames, gen_listeners);
            return {*this, id};
        }
    };

    struct IPrototypeGen {
        virtual typename Frame::GameObject generate(GameStateGenClient<types<GEN...>>::Frame&) = 0;
    };

    template<typename ...Ts>
    struct PrototypeGen {
        PrototypeGen() = default;
        template<typename ...Attrs>
        PrototypeGen(types<Attrs...>) {}
        typename Frame::GameObject generate(GameStateGenClient<types<GEN...>>::Frame& frame) override {
            return frame.template gen<Ts...>();
        }
    };

    auto get_frame() {
        return Frame{gen_clients, gen_listeners};
    }
};

