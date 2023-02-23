#include <GameState/GameStateClient.hpp>

#include <atomic>

size_t get_next_game_object_id() {
    static std::atomic_size_t id{1};
    return id.fetch_add(1);
}
