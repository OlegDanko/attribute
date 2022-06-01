#pragma once

#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <mutex>

template<typename FRAME_DATA>
class BookmarkedQueue {
    struct Bookmark;
    struct Frame;

    class Item {
    public:
        class Visitor {
            std::function<void(FRAME_DATA&)> fn;
            const size_t client_id;;
        public:
            Visitor(size_t id, std::function<void(FRAME_DATA&)> fn) : client_id(id), fn(fn) {}
            void visit(Bookmark&) const {/*bookmarks are ignored*/}
            void visit(Frame&) const;
        };
        virtual void accept(const Visitor&) = 0;
        virtual size_t get_origin_id() const = 0;
    };

    struct ItemBase : Item {
        const size_t origin_id;
        ItemBase(size_t id) : origin_id(id) {}
        size_t get_origin_id() const override { return 0; }
    };

    struct Bookmark : ItemBase {
        void accept(const typename Item::Visitor& v) override { v.visit(*this); }
        Bookmark(size_t id) : ItemBase(id) {}
    };

    struct Frame : ItemBase {
        FRAME_DATA data;
        void accept(const typename Item::Visitor& v) override { v.visit(*this); }
        Frame(size_t id) : ItemBase(id) {}
    };


    using item_list_t = std::list<std::unique_ptr<Item>>;
    using item_it_t = typename item_list_t::iterator;
    item_list_t items;

    using bookmark_list_t = std::list<item_it_t>;
    using bookmark_it_t = typename bookmark_list_t::iterator;
    bookmark_list_t bookmarks;

    struct bookmark_range {
        bookmark_it_t begin;
        std::optional<bookmark_it_t> end;
    };
    using bookmark_range_map_t = std::unordered_map<size_t, bookmark_range>;
    using bookmark_range_map_it_t = typename bookmark_range_map_t::iterator;
    bookmark_range_map_t bookmark_range_by_id_map;

    std::mutex mtx;

    auto create_bookmark(size_t id) {
        return bookmarks.insert(bookmarks.end(), items.insert(items.end(), std::make_unique<Bookmark>(id)));
    }

    void dismiss_bookmark(bookmark_it_t bm_it) {
        bookmarks.erase(bm_it);
        items.erase(items.begin(), *bookmarks.begin());
    }

    void dismiss_bookmark_range(bookmark_range_map_it_t bm_it) {
        std::lock_guard lk(mtx);
        auto& [_, bm] = *bm_it;
        dismiss_bookmark(bm.begin);
        bm = {bm.end.value(), std::nullopt};
    }

    bookmark_range_map_it_t get_bookmark_range(size_t id) {
        auto bm_it = bookmark_range_by_id_map.find(id);

        if(bm_it == bookmark_range_by_id_map.end())
            throw std::logic_error("Requested bookmark range for BookmarkedQueue client with unknown ID");

        auto& [_, bm] = *bm_it;

        if(bm.end.has_value())
            throw std::logic_error("Attempting to read range twice at the same time with the same BookmarkedQueue client ID");

        bm.end = create_bookmark(id);

        return bm_it;
    }

    class ItemsProvider {
        BookmarkedQueue& q;
        bookmark_range_map_it_t bm_it;
    public:
        ItemsProvider(BookmarkedQueue& q, bookmark_range_map_it_t bm_it)
            : q(q)
            , bm_it(bm_it) {}
        ~ItemsProvider() {
            q.dismiss_bookmark_range(bm_it);
        }

        item_it_t begin() {
            return *bm_it->second.begin;
        }
        item_it_t end() {
            auto end_opt = bm_it->second.end;

            if(!end_opt.has_value())
                throw std::logic_error("Missing frame end iterator");

            return *end_opt.value();
        }
    };

    ItemsProvider read(size_t id) {
        std::lock_guard lk(mtx);
        return {*this, get_bookmark_range(id)};
    }

    void add_frame(std::unique_ptr<Frame> frame) {
        std::lock_guard lk(mtx);
        items.push_back(std::move(frame));
    }

    class Client {
        BookmarkedQueue& q;
        size_t id;

        class FrameHolder {
            BookmarkedQueue& q;
            std::unique_ptr<Frame> frame;
        public:
            FrameHolder(BookmarkedQueue& q, size_t id)
                : q(q)
                , frame(std::make_unique<Frame>(id))
            {}
            ~FrameHolder() { q.add_frame(std::move(frame)); }
            FRAME_DATA& operator*() { return frame->data; }
            FRAME_DATA& operator->() { return frame->data; }
        };

    public:
        Client(BookmarkedQueue& q, size_t id) : q(q), id(id) {}

        FrameHolder create_frame() {
            return {q, id};
        }
        void serve_frame(std::function<void(const FRAME_DATA&)> fn) {
            for(const auto& frame : q.read(id))
                frame->accept({id, fn});
        }
    };

public:
    Client create_client() {
        static size_t id = 0;

        std::lock_guard lk(mtx);
        ++id;
        bookmark_range_by_id_map[id] = {
            create_bookmark(id),
            std::nullopt
        };
        return Client{*this, id};
    }
};

template<typename T>
void BookmarkedQueue<T>::Item::Visitor::visit(Frame& e) const {
    if(e.origin_id != client_id) fn(e.data);
}
