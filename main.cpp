#include <Attribute.hpp>
#include <AttributesList.hpp>
#include <BookmarkedQueue.hpp>
#include <StateBufferQueue/StateBufferQueue.hpp>
#include <StateBufferQueue/ExampleStateBufferQueueMaps.hpp>
#include <vector>

#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <mutex>

#include <utils/utils.hpp>
#include <utils/IdInc.hpp>

#define COMMENT(X)

COMMENT(
| id | Transform | KeyController | AIController | MovementApplier | INetObject | Mover | Renderable |
|    |           |               |              | Input     AI    | OUT   IN   |       |            |
|----|-----------|---------------|--------------|-----------------|------------|-------|------------|
| 1  | X         | X             |              | X               | X          | X     | X          | <- Game object controlled by a local client
| 1  | X         |               | X            |           X     | X          | X     | X          | <- Game object controlled by local simulation
| 2  | X         |               |              |                 |       X    | X     | X          | <- Game object controlled remotely
)

template<typename T, typename...Ts> void test_fn() { test_fn<T>(); test_fn<Ts...>();}
template<> void test_fn<int>() { std::cout << "int\n"; }

template<typename>
struct test_types;

template<typename...Ts>
struct test_types<types<Ts...>> {
    static void fn() { test_fn<Ts...>(); }
};

class KeyController : public Attribute<> {};

class AIController : public Attribute<> {};

class Mover : public Attribute<> {};

class IMovementApplier : public Attribute<> {};

class MovementNetObject : public Attribute<void, types<Mover>> {};

class KeyMovementApplier : public Attribute<IMovementApplier, types<KeyController, Mover, MovementNetObject>> {};

class AIMovementApplier : public Attribute<IMovementApplier, types<AIController, Mover, MovementNetObject>> {};

class Renderable : public Attribute<> {};

struct vec2 {
    float x, y;
    void operator +=(const vec2& v) {
        x += v.x;
        y += v.y;
    }
    vec2 operator +(const vec2& v) const {
        return {x+v.x, y+v.y};
    }
    vec2 operator *(const vec2& v) const {
        return {x*v.x, y*v.y};
    }
    bool operator ==(const vec2& v) const {
        return x == v.x && y == v.y;
    }
    bool operator !=(const vec2& v) const {
        return !(*this == v);
    }
};

::std::ostream& operator<<(::std::ostream& s, vec2 v) {
    return s << "{" << v.x << ", " << v.y << "}";
};

struct GameObject {
    vec2 pos;
    vec2 dir;
};

using game_state_queue_t = StateBufferQueue<HolderMap, AccessorMap, GameObject>;

struct GameContext {
    game_state_queue_t game_state_queue;
};

void simple_state_updating_reading_demo() {
    using game_state_queue_t = StateBufferQueue<HolderMap, AccessorMap, GameObject>;

    struct PositionUpdate {
        game_state_queue_t::Client q_cli;
        std::vector<size_t>& ids;
        void run(std::atomic_bool& running, std::chrono::milliseconds frame_time) {
            while(running.load()) {
                std::cout << "updating\n";
                with(auto rb = q_cli.get_read_buffer()) {
                    auto wb = q_cli.get_write_buffer();

                    for(auto id : ids) {
                        const auto* prev = rb->read(id);
                        if(prev->dir == vec2{0, 0}) continue;
                        auto* curr = wb->gen(id);
                        *curr = {prev->pos + prev->dir, prev->dir};
                    }
                }
                std::this_thread::sleep_for(frame_time);
            }
        }
    };

    struct PositionConsumer {
        const game_state_queue_t::Client q_cli;
        std::vector<size_t>& ids;
        void run(std::atomic_bool& running, std::chrono::milliseconds frame_time) {
            while(running.load()) {
                std::cout << "consuming\n";
                with(auto rb = q_cli.get_read_buffer()) {
                    for(auto id : ids) {
                        const auto* obj = rb->read(id);
                        std::cout << "Obj " << id << " pos " << obj->pos << " dir " << obj->dir << "\n";
                    }
                }
                std::this_thread::sleep_for(frame_time);
            }
        }
    };
    game_state_queue_t game_state_queue;

    std::vector<size_t> ids;
    with(auto wb = game_state_queue.create_client().get_write_buffer()) {
        size_t id = 0;
        ids.push_back(++id);
        *wb->gen(ids.back()) = {{10, 10}, {1, 1}};
        ids.push_back(++id);
        *wb->gen(ids.back()) = {{-10, -10}, {1, 1}};
    }

    PositionUpdate pos_update{game_state_queue, ids};
    PositionConsumer pos_consume{game_state_queue, ids};

    std::atomic_bool running = true;

    std::thread t1([&](){ pos_update.run(running, std::chrono::milliseconds{300});});
    std::thread t2([&](){ pos_consume.run(running, std::chrono::milliseconds{1000});});

    std::this_thread::sleep_for(std::chrono::seconds(5));
    running = false;

    t1.join();
    t2.join();
}


void test_state_buffer_queue();
void test_Bookmarked_queue();

int main()
{
    simple_state_updating_reading_demo();

    //test_state_buffer_queue();
    //test_Bookmarked_queue();

    return 0;
}




























void test_state_buffer_queue() {
    struct S{ char c; };
    using StateQueue = StateBufferQueue<HolderMap, AccessorMap, S>;
    StateQueue q;
    auto cl1 = q.create_client();
    auto cl2 = q.create_client();

    auto read_item = [](StateQueue::Client::ReadBuffer& rb, size_t id) {
        if(auto* a = rb->read(1); a != nullptr)
            std::cout << "\tFound character " << a->c << "\n";
        else
            std::cout << "\tNone was found\n";
    };

    std::cout << "Acquiring new read buffer\n";
    with(auto rb = cl2.get_read_buffer()) {
        std::cout << "Acquiring new write buffer\n";
        with(auto wb = cl1.get_write_buffer()) {
            std::cout << "Reading item before it was written to the write buffer, expecting none\n";
            read_item(rb, 1);

            std::cout << "Creating new item in the write buffer, writing 'a'\n";
            wb->gen(1)->c = 'a';

            std::cout << "Reading item after it was written to the write buffer, expecting none\n";
            read_item(rb, 1);
        }
        std::cout << "Releasing write buffer\n";
    }
    std::cout << "Releasing read buffer\n";

    std::cout << "Acquiring new read buffer\n";
    with(auto rb = cl2.get_read_buffer()) {
        std::cout << "Reading item after previous write buffer was released, expecting 'a'\n";
        read_item(rb, 1);

        std::cout << "Acquiring write buffer, updating item to 'b', releasing write buffer\n";
        with(auto wb = cl1.get_write_buffer()) {
            auto* c = wb->gen(1);
            c->c = 'b';
        }

        std::cout << "Reading item after write buffer was released, expecting 'a'\n";
        read_item(rb, 1);
    }

    std::cout << "Acquiring new read buffer\n";
    with(auto rb = cl2.get_read_buffer()) {
        std::cout << "Reading item, expecting 'b'\n";
        read_item(rb, 1);
    }
}

void test_Bookmarked_queue() {
    BookmarkedQueue<int> queue;
    auto c1 = queue.create_client();
    auto c2 = queue.create_client();
    std::mutex mtx;
    std::atomic_bool running{true};

    auto thread_fn = [&mtx, &running](auto& client, const std::string& name, int data, int sleep_ms){
        while(running.load()) {
            with(std::lock_guard lk(mtx)) {
                std::cout << "Reading for " << name << ":" << std::endl;
                client.serve_frame([&name, &mtx](const auto& d){
                    std::cout << "\t" << name << " received " << d << std::endl;
                });
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms/2));

            with(std::lock_guard lk(mtx))
                    std::cout << "Writing for " << name << std::endl;

            *client.create_frame() = data;

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms/2));
        }
    };

    std::thread t1([&](){ thread_fn(c1, "client 1", 1, 500); });
    std::thread t2([&](){ thread_fn(c2, "client 2", 2, 1000); });

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    running = false;
    t1.join();
    t2.join();
}

