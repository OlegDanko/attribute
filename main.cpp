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

template <typename T>
struct Struct
{
    Struct() {}
};

using utl_prf::index_of_type_v;
using utl_prf::unique_types;

int main() {
    SubTypesChain<Struct, int> chain;

    using types_t = unique_types<int, float, double, char, float, int>::types_;
    using types_2 = unique_types<int, types<float, double, char, float, int>>::types_;

//    static_assert(std::is_same_v<types_t, types_2>);

    std::cout << "int is in position " << index_of_type_v<int, types_t> << std::endl;
    std::cout << "float is in position " << index_of_type_v<float, types_t> << std::endl;
    std::cout << "double is in posistion " << index_of_type_v<double, types_t> << std::endl;
    std::cout << "char is in position " << index_of_type_v<char, types_t> << std::endl;
//    std::cout << "char is in position " << index_of_v<std::string, types_t> << std::endl;

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
