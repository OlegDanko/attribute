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

        template<typename T>
        void serve_game_objects(std::function<void(GameObject)> fn) {
            if constexpr (is_present_v<T, MOD...>) {
                return (*std::get<index_of_v<T, MOD...>>(mod_frames))->read_serve(
                            [this, fn](auto id, const auto&){ fn(get(id)); });
            }

            if constexpr (is_present_v<T, READ...>) {
                return (*std::get<index_of_v<T, READ...>>(read_frames))->read_serve(
                            [this, fn](auto id, const auto&){ fn(get(id)); });
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

template<typename ...GEN>
struct GameStateGenClient<types<GEN...>> {
    using gen_clients_t = typename type_apply<GenFrameProvider, GEN...>::types_::tpl;
    gen_clients_t gen_clients;

    GameStateGenClient(GameState<types<GEN...>>& gs) : gen_clients(std::move(gs.get_gen_clients())) {}

    struct Frame {
        decltype(collect_frames(gen_clients)) gen_frames;

        Frame(gen_clients_t& gen_clients)
            : gen_frames(std::move(collect_frames(gen_clients))) {}


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
            static void gen(size_t id, decltype(gen_frames)& frames) {
                (*std::get<index_of_v<T, GEN...>>(frames))->gen(id);
            }
        };

        template<typename T, typename ...Ts>
        struct generator<T, Ts...> {
            static void gen(size_t id, decltype(gen_frames)& frames) {
                (*std::get<index_of_v<T, GEN...>>(frames))->gen(id);
                generator<Ts...>::gen(id, frames);
            }
        };

        template<typename ...Ts>
        GameObject gen(types<Ts...> = types<Ts...>()) {
            auto id = game_object_id::get();
            generator<Ts...>::gen(id, gen_frames);
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
        return Frame{gen_clients};
    }
};

