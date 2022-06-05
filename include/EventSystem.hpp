#include <iostream>
#include <queue>
#include <memory>
#include <chrono>
#include <list>
#include <map>
#include <set>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <math.h>

#include <algorithm>

struct event {
    size_t id;
    std::string msg;
};

struct event_system {
    struct event_holder {
        event e;
        std::unique_ptr<std::atomic_int> count;
        event_holder(event_holder&&) = default;
        event_holder(event e) : e(e), count(std::make_unique<std::atomic_int>(0)) {}
        ~event_holder() {
            if(!e.msg.empty())
            std::cout << "deleting event holder for " << e.id << " with message " << e.msg << "\n";
        }
    };

    using event_list_t = std::list<event_holder>;
    using event_list_it_t = event_list_t::iterator;
    event_list_t e_list;
    std::unordered_map<size_t, std::unordered_set<size_t>> consumers_ids_by_event_id;
    std::unordered_map<size_t, std::queue<event_list_it_t>> events_by_consumer_id;

    void decrement_event(event_list_it_t it) {
        --(*it->count);
        if(*it->count <= 0) {
            e_list.erase(it);
        }
    }

    void add_event(event e) {
        if(consumers_ids_by_event_id[e.id].empty())
            return;

        e_list.emplace_back(event_holder{e});
        auto e_it = std::prev(e_list.end());
        for(auto c_id : consumers_ids_by_event_id[e_it->e.id]) {
            events_by_consumer_id[c_id].push(e_it);
            ++(*e_it->count);
        }
    }
    void add_consumer(size_t e_id, size_t c_id) {
            consumers_ids_by_event_id[e_id].insert(c_id);
    }
    void consume_events(size_t c_id, std::function<void(const event&)> fn) {
        auto q = std::move(events_by_consumer_id[c_id]);
        events_by_consumer_id[c_id] = {};

        while(!q.empty()) {
            auto it = q.front();
            q.pop();
            fn(it->e);
            decrement_event(it);
        }
    }
};

int main() {
    auto mk_consume_event_fn = [](size_t id){
        return [id](const event& e) {
            std::cout << id << " is consuming event " << e.id << " with message " << e.msg << "\n";
        };
    };

    event_system s;
    s.add_event({1, "1 - first"});
    s.add_consumer(1, 1);

    std::cout << "\n";
    s.add_event({1, "1 - second"});
    s.consume_events(1, mk_consume_event_fn(1));

    s.add_consumer(1, 2);
    std::cout << "\n";
    s.add_event({1, "1 - third"});
    s.consume_events(1, mk_consume_event_fn(1));
    s.consume_events(2, mk_consume_event_fn(2));


    s.add_consumer(2, 1);
    s.add_consumer(3, 2);

    s.add_event({1, "1 - fourth"});
    s.add_event({2, "2 - first"});
    s.add_event({2, "2 - second"});
    s.add_event({3, "3 - first"});
    s.add_event({3, "3 - second"});

    std::cout << "\n";
    s.consume_events(1, mk_consume_event_fn(1));
    s.consume_events(2, mk_consume_event_fn(2));
    s.consume_events(3, mk_consume_event_fn(3));
    //s.add_event({2, "hello 2"});

    return 0;
}
