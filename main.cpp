#include <utils/utils.hpp>
#include <utils/IdInc.hpp>

#include <vector>
#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <mutex>

#include <GameState/GameState.hpp>
#include <GameState/GameStateClient.hpp>

template<typename T>
using u_ptr = std::unique_ptr<ModFrameDataHolder<T>>;

int main() {
    GameState<types<int, double>> gs;
    auto mod_providers = gs.get_mod_providers<int>();
    auto mod_frames = mod_providers.get_frames();
    auto read_providers = gs.get_read_providers<int>();
    auto read_frames = read_providers.get_frames();

    GameStateGenClient<types<int, double>> gen_cli(gs);

    size_t id = 0;
    with(auto f = gen_cli.get_frame()) {
        auto go = f.gen<int, double>();
        id = go.get_id();
        *go.get_attr<int>() = 10;
        *go.get_attr<double>() = 20.0f;
    }

    GameStateClient<types<double, int>, types<>> mod_cli(gs);
    with(auto f = mod_cli.get_frame()) {
        auto go = f.get(id);
        *go.get_attr<int>() *= 10;
        *go.get_attr<double>() *= 10;
    }

    GameStateClient<types<>, types<double, int>> read_cli(gs);
    with(auto f = read_cli.get_frame()) {
        auto go = f.get(id);
        std::cout << *go.read_attr<int>() << " " << *go.read_attr<double>() << std::endl;
    }

    return 0;
}
