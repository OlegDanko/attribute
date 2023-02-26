#pragma once

#include "GameState_decl.hpp"
#include "GameState.hpp"
#include "IObjectGenListener.hpp"

template<typename ...MOD, typename ... READ>
struct GameStateClient<types<MOD...>, types<READ...>> {
    using mod_clients_t = FrameProviders<ModFrameProvider, MOD...>;
    using read_clients_t = FrameProviders<ReadFrameProvider, READ...>;
    mod_clients_t mod_clients;
    read_clients_t read_clients;

    template<typename ...Ts>
    GameStateClient(GameState<Ts...>& gs)
        : mod_clients(gs.template get_mod_providers<MOD...>())
        , read_clients(gs.template get_read_providers<READ...>()) {}

    struct Frame {
        SubTypesChain<ModFrameDataHolder_ptr_t, MOD...> mod_frames;
        SubTypesChain<ReadFrameDataHolder_ptr_t, READ...> read_frames;

        Frame(mod_clients_t& mod_clients, read_clients_t& read_clients)
            : mod_frames(mod_clients.get_frames())
            , read_frames(read_clients.get_frames()) {}

        template<typename T>
        const T* read(size_t id) const {
            if constexpr (is_present_v<T, MOD...>) {
                return (*mod_frames.template get<T>())->read(id);
            }
            else if constexpr (is_present_v<T, READ...>) {
                return (*read_frames.template get<T>())->read(id);
            }
            return nullptr;
        }

        template<typename T>
        T* get(size_t id) {
            if constexpr (is_present_v<T, MOD...>) {
                return (*mod_frames.template get<T>())->get(id);
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
                auto [begin, end] = (*mod_frames.template get<T>())
                        ->const_iter_range();
                auto [b, e] = go_citers_from_attr_citers(begin, end);
                return utl_prf::iterable(b, e);
            }
            if constexpr (is_present_v<T, READ...>) {
                auto [begin, end] = (*read_frames.template get<T>())
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

size_t get_next_game_object_id();

template<typename ...GEN>
struct GameStateGenClient<types<GEN...>> {
    using gen_clients_t = FrameProviders<GenFrameProvider, GEN...>;
    gen_clients_t gen_clients;

    using listeners_register_t = ObjectGenListenerRegister<GEN...>;
    listeners_register_t gen_listeners;

    template<typename T>
    void add_listener(IObjectGenListener<T>* l) {
        gen_listeners.add(l);
    }

    GameStateGenClient(GameState<types<GEN...>>& gs)
        : gen_clients(gs.get_gen_providers()) {}

    struct Frame {
        SubTypesChain<GenFrameDataHolder_ptr_t, GEN...> gen_frames;
        listeners_register_t& gen_listeners;

        Frame(gen_clients_t& gen_clients, listeners_register_t& gen_listeners)
            : gen_frames(gen_clients.get_frames())
            , gen_listeners(gen_listeners) {}

        template<typename T>
        T* get(size_t id) {
            if constexpr (is_present_v<T, GEN...>) {
                return (*gen_frames.template get<T>())->get(id);
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

        template<typename T>
        void gen_(size_t id) {
            (*gen_frames.template get<T>())->gen(id);
        }

        template<typename ...Ts>
        GameObject gen(types<Ts...> = types<Ts...>()) {
            auto id = get_next_game_object_id();
            (gen_<Ts>(id),...);
            gen_listeners.template on_generated<Ts...>(id);
            return {*this, id};
        }
    };

    struct IPrototypeGen {
        virtual typename Frame::GameObject
        generate(GameStateGenClient<types<GEN...>>::Frame&) = 0;
    };

    template<typename ...Ts>
    struct PrototypeGen {
        PrototypeGen() = default;
        template<typename ...Attrs>
        PrototypeGen(types<Attrs...>) {}
        typename Frame::GameObject
        generate(GameStateGenClient<types<GEN...>>::Frame& frame) override {
            return frame.template gen<Ts...>();
        }
    };

    auto get_frame() {
        return Frame{gen_clients, gen_listeners};
    }
};

