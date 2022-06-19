#include <Attribute.hpp>
#include <AttributesList.hpp>
#include <BookmarkedQueue.hpp>
#include <StateBufferQueue/StateBufferQueue.hpp>
#include <StateBufferQueue/ExampleMaps.hpp>
#include <EventHandling/AttributeEventHandling.hpp>

#include <StateFrameQueue/StateFrameQueue.hpp>

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

#define COMMENT(X)

COMMENT(
| id | Transform | KeyController     | AIController | MovementApplier | INetObject | Mover | Renderable |
|    |           | Agent   FreeFloat |              | Input     AI    | OUT   IN   |       |            |
|----|-----------|-------------------|--------------|-----------------|------------|-------|------------|
| 1  | X         | X                 |              | X               | X          | X     | X          | <- Game object controlled by a local client
| 1  | X         |                   | X            |           X     | X          | X     | X          | <- Game object controlled by local simulation
| 2  | X         |                   |              |                 |       X    | X     | X          | <- Game object controlled remotely
)

template<typename T, typename...Ts> void test_fn() { test_fn<T>(); test_fn<Ts...>();}
template<> void test_fn<int>() { std::cout << "int\n"; }

template<typename>
struct test_types;

template<typename...Ts>
struct test_types<types<Ts...>> {
    static void fn() { test_fn<Ts...>(); }
};

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


struct vec3 {
    float x, y, z;
    void operator +=(const vec3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    vec3 operator +(const vec3& v) const {
        return {x+v.x, y+v.y, z+v.z};
    }
    vec3 operator *(const vec3& v) const {
        return {x*v.x, y*v.y, z*v.z};
    }
    bool operator ==(const vec3& v) const {
        return x == v.x && y == v.y;
    }
    bool operator !=(const vec3& v) const {
        return !(*this == v);
    }
};

::std::ostream& operator<<(::std::ostream& s, vec3 v) {
    return s << "{" << v.x << ", " << v.y << ", " << v.z << "}";
};

class Transform : public Attribute<void, types<>> {
    using base_t = Attribute<void, types<>>;
    vec3 position;
    vec3 rotation;
    vec3 scale;

    Transform(size_t id) : base_t(id) {}
    Transform(const Transform& that)
        : base_t(that.get_id())
        , position(that.position)
        , rotation(that.rotation)
        , scale(that.scale) {}
    Transform(Transform&& that)
        : base_t(that.get_id())
        , position(that.position)
        , rotation(that.rotation)
        , scale(that.scale) {}

    struct : AttributeEventDispatcher<vec3> {} position_changed_event;
    void on_position_changed(vec3 val) { position_changed_event.stage(get_id(), val); }
    struct : AttributeEventDispatcher<vec3> {} rotation_changed_event;
    void on_rotation_changed(vec3 val) { rotation_changed_event.stage(get_id(), val); }
    struct : AttributeEventDispatcher<vec3> {} scale_changed_event;
    void on_scale_changed(vec3 val) { scale_changed_event.stage(get_id(), val); }


    void set_position(vec3 val) { position = val; on_position_changed(val); }
    void set_rotation(vec3 val) { rotation = val; on_rotation_changed(val); }
    void set_scale(vec3 val) { scale = val; on_scale_changed(val); }

    void position_changed_callback(vec3 val) { set_position(val); }
    void rotation_changed_callback(vec3 val) { set_rotation(val); }
    void scale_changed_callback(vec3 val) { set_scale(val); }
};

class KeyController : public Attribute<> {};

class AIController : public Attribute<> {};

class Mover : public Attribute<> {};

class MovementNetObject : public Attribute<void, types<Mover>> {};

class IMovementApplier : public Attribute<> {};

class KeyMovementApplier : public Attribute<IMovementApplier, types<KeyController, Mover, MovementNetObject>> {};

class AIMovementApplier : public Attribute<IMovementApplier, types<AIController, Mover, MovementNetObject>> {};

class Renderable : public Attribute<> {};

class GameObjectCreator {

};


//void simple_state_updating_reading_demo();
//void test_attribute_enevt_handling();
//void test_state_buffer_queue();
//void test_Bookmarked_queue();
//void test_event_handling();

int main()
{
    StateFrameQueue<int> gameStateQueue;

    auto reader = gameStateQueue.get_read_provider();
    auto generator = gameStateQueue.get_gen_provider();
    auto modifier = gameStateQueue.get_mod_provider();

    auto read_all = [&reader]{
        with(auto data_holder = reader.get()) {
            std::cout << "Reading:\n";
            data_holder->read_serve([](auto id, const auto& T){
                std::cout << id << " - " << T << "\n";
            });
        }
    };

    with(auto frame_holder = generator.get()) {
        *frame_holder->gen(1) = 10;
        read_all();
    }

    read_all();
    with(auto frame_holder = modifier.get()) {
        *frame_holder->get(1) = 20;
        read_all();
    }
    read_all();

    with(auto frame_holder = modifier.get()) {
        with(auto frame_holder = generator.get()) {
            *frame_holder->gen(2) = 100;
        }
        *frame_holder->get(1) = 30;
    }
    read_all();
    with(auto frame_holder = generator.get()) {
        with(auto frame_holder = modifier.get()) {
            *frame_holder->get(1) = 40;
        }
        *frame_holder->gen(3) = 1000;
    }
    read_all();


//    simple_state_updating_reading_demo();
//    test_attribute_enevt_handling();
//    test_event_handling();
//    simple_state_updating_reading_demo();
//    test_state_buffer_queue();
//    test_Bookmarked_queue();

    return 0;
}




























/*
void simple_state_updating_reading_demo() {
    struct ExampleGameObject {
        vec2 pos;
        vec2 dir;
    };

    using game_state_queue_t = StateBufferQueue<HolderMap, AccessorMap, ExampleGameObject>;


    struct GameContext {
        game_state_queue_t game_state_queue;

        struct obj_generator : std::enable_shared_from_this<obj_generator> {
            game_state_queue_t::Client::WriteBuffer wb;

            struct obj_holder {
                std::shared_ptr<obj_generator> sp;
                ExampleGameObject* obj;
                size_t id;
                ExampleGameObject& operator->(){ return *obj; }
                ExampleGameObject& operator*(){ return *obj; }
                ~obj_holder() { std::cout << "deleting obj_holder\n"; }
            };

            obj_holder gen() {
                static size_t id = 0;
                ++id;
                return {
                    shared_from_this(),
                    wb->gen(id),
                    id
                };
            }
            obj_generator(game_state_queue_t::Client::WriteBuffer wb) : wb(std::move(wb)) {}
            ~obj_generator() { std::cout << "deleting obj_generator\n"; }
        };
        std::shared_ptr<obj_generator> get_generator() {
            return std::make_shared<obj_generator>(game_state_queue.create_client().get_write_buffer());
        }
    };

    using game_state_queue_t = StateBufferQueue<HolderMap, AccessorMap, ExampleGameObject>;

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

    GameContext gc;
    std::vector<size_t> ids;


    with(auto generator = gc.get_generator()) {
        auto obj = generator->gen();
        ids.push_back(obj.id);
        *obj = {{10, 10}, {1, 1}};
        obj = generator->gen();
        ids.push_back(obj.id);
        *obj = {{-10, -10}, {1, 1}};
    }

    PositionUpdate pos_update{gc.game_state_queue, ids};
    PositionConsumer pos_consume{gc.game_state_queue, ids};

    std::atomic_bool running = true;

    std::thread t1([&](){ pos_update.run(running, std::chrono::milliseconds{3});});
    std::thread t2([&](){ pos_consume.run(running, std::chrono::milliseconds{10});});

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    running = false;

    t1.join();
    t2.join();
}

struct EventProducer {
    size_t id;

    struct : AttributeEventDispatcher<int>{} static event_a;
    void on_a(int v) { event_a.stage(id, v); }

    struct : AttributeEventDispatcher<int, double>{} static event_b;
    void on_b(int v1, double v2) { event_b.stage(id, v1, v2); }

    static std::vector<ITriggerable*> get_event_sources() { return {&event_a, &event_b}; }
};

decltype(EventProducer::event_a) EventProducer::event_a;
decltype(EventProducer::event_b) EventProducer::event_b;

struct EventConsumer {
    size_t id;
    void callback_a(int v) {
        std::cout << "consumer " << id << " received event a with val " << v << std::endl;
    }
    void callback_b(int v_i, double v_d) {
        std::cout << "consumer " << id << " received event b with vals {" << v_i << ", " << v_d << "}" << std::endl;
    }
};

void test_attribute_enevt_handling() {
    std::unordered_map<size_t, EventProducer> producers;
    std::unordered_map<size_t, EventConsumer> consumers;

    producers[1] = EventProducer{.id = 1};
    consumers[1] = EventConsumer{.id = 1};
    producers[2] = EventProducer{.id = 2};
    consumers[2] = EventConsumer{.id = 2};

    struct Provider : TypeByIdProvider<EventConsumer>{
        using map_t = std::unordered_map<size_t, EventConsumer>;
        map_t& map;
        Provider(map_t& m) : map(m){}
        EventConsumer& get(size_t id) override { return map[id]; }
    };
    Provider provider{consumers};


    std::vector<std::unique_ptr<IListener>> listeners;
    listeners.push_back(bind_event_listener(EventProducer::event_a, provider, &EventConsumer::callback_a));
    listeners.push_back(bind_event_listener(EventProducer::event_b, provider, &EventConsumer::callback_b));

    auto flush = [&]() {
        std::cout << "\n\tNew frame\n";
        for(auto l : EventProducer::get_event_sources()) l->trigger();
        for(auto& l : listeners) l->serve_events();
    };

    producers[1].on_a(10);
    flush();

    producers[2].on_b(10, 3.14);
    flush();


    producers[1].on_a(10);
    producers[2].on_a(20);
    producers[1].on_b(100, 30.14);
    producers[2].on_b(200, 300.14);
    flush();

}

void test_event_handling() {
    struct DemoListener : EventListener<std::string> {
        size_t id;
        void serve_event(event_t& e) override {
            std::cout << "Target: " << id << ", event: " << e.id
                      << ", msg: " << std::get<0>(e.args) << std::endl;
        }
    };

    DemoListener l1;//{.id = 1};
    l1.id = 1;
    DemoListener l2;//{.id = 2};
    l2.id = 2;

    EventDispatcher<std::string> s;

    auto trigger_evs = [&](std::string s1, std::string s2, std::string s3) {
        s.stage(std::make_shared<Event<std::string>>(1, s1));
        s.stage(std::make_shared<Event<std::string>>(2, s2));
        s.stage(std::make_shared<Event<std::string>>(3, s3));
        s.trigger();

        l1.serve_events();
        l2.serve_events();
    };

    trigger_evs("1",
                "2",
                "3");

    s.reg_listener(&l1);

    trigger_evs("10",
                "20",
                "30");

    s.reg_listener(&l2);

    trigger_evs("100",
                "200",
                "300");
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
*/
