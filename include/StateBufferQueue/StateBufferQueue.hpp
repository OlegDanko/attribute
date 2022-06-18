#pragma once

#include <utils/utils.hpp>
#include <utils/IdInc.hpp>

#include <type_traits>
#include <memory>
#include <unordered_set>
#include <list>
#include <mutex>

template<template<typename> typename HolderMap_t, template<typename> typename AccessorMap_t, typename T>
class StateBufferQueue {
    class StateBuffer {
        friend class StateBufferQueue;

        size_t id;
        HolderMap_t<T> holder;
        AccessorMap_t<T> accessor;

        void update(HolderMap_t<T>&& from) {
            accessor.update(from);
            holder.merge_from(std::move(from));
        }

        void merge_from(StateBuffer&& from) {
            holder.merge_from(std::move(from.holder));
            accessor = std::move(from.accessor);
        }
        void set_prev(StateBuffer& prev) {
            accessor = prev.accessor;
            accessor.update(holder);
        }
        template<typename T_>
        static constexpr void check_type() {
            static_assert(std::is_base_of_v<T, T_>, "Attempting to read object of unrelated type");
        }
    public:
        StateBuffer(size_t id) : id(id) {}
        StateBuffer(StateBuffer&&) = default;
        StateBuffer(const StateBuffer&) = delete;
        explicit StateBuffer(HolderMap_t<T>&& h) : holder(std::move(h)) {}

        template<typename T_ = T>
        T_* gen(size_t id) {
            return holder.template gen<T_>(id);
        }
        template<typename T_ = T>
        const T_* read(size_t id) const {
            return accessor.template read<T_>(id);
        }
        auto begin() const { return accessor.begin(); }
        auto end() const { return accessor.end(); }
    };

    using buffer_list_item_t = std::pair<std::unordered_set<size_t>, StateBuffer>;
    using buffer_list_t = std::list<buffer_list_item_t>;
    using buffer_list_it_t = typename buffer_list_t::iterator;
    buffer_list_t buffers;
    std::mutex mtx;

    bool is_free(buffer_list_it_t buf_it) { return buf_it->first.empty(); }

    void merge_buffers(buffer_list_it_t buf_it_into, buffer_list_it_t buf_it_from) {
        buf_it_into->second.merge_from(std::move(buf_it_from->second));
        buffers.erase(buf_it_from);
    }

    void try_merge_next(buffer_list_it_t buf_it) {
        if(auto next = std::next(buf_it); next != buffers.end() && is_free(next)) {
            merge_buffers(buf_it, next);
        }
    }

    void try_merge_prev(buffer_list_it_t buf_it) {
        if(buffers.begin() == buf_it) return;
        if(auto prev = std::prev(buf_it); is_free(prev))
            merge_buffers(prev, buf_it);
    }

    buffer_list_it_t acquire_read_buffer(size_t id) {
        auto buf = std::prev(buffers.end());
        buf->first.insert(id);
        return buf;
    }

    void release_read_buffer(size_t id, buffer_list_it_t buf_it) {
        std::lock_guard lk(mtx);
        buf_it->first.erase(id);

        if(!is_free(buf_it))
            return;

        try_merge_next(buf_it);
        try_merge_prev(buf_it);
    }

    void add_buffer(HolderMap_t<T>&& buf) {
        std::lock_guard lk(mtx);
        auto mk_item_fn = utl_prf::pair_maker<buffer_list_item_t>::make;

        if(buffers.end() == buffers.begin()) {
            buffers.emplace_back(mk_item_fn({}, StateBuffer(std::move(buf))));
            return;
        }
        auto latest = std::prev(buffers.end());

        if(is_free(latest)) {
            latest->second.update(std::move(buf));
            return;
        }

        StateBuffer s_buf(std::move(buf));
        s_buf.set_prev(latest->second);
        buffers.emplace_back(mk_item_fn({}, std::move(s_buf)));
    }

public:
    class Client {
        static utl_prf::IdInc<size_t> ids;
        StateBufferQueue &q;
        size_t id;

    public:
        class ReadBuffer {
            friend class Client;
            StateBufferQueue &q;
            size_t id;
            buffer_list_it_t it;
        public:
            ReadBuffer(StateBufferQueue &q, size_t id) : q(q), id(id), it(q.acquire_read_buffer(id)) {}
            ~ReadBuffer() { q.release_read_buffer(id, it); }
            ReadBuffer(ReadBuffer&&) = default;
            ReadBuffer(const ReadBuffer&) = delete;
            ReadBuffer& operator=(const ReadBuffer&) = delete;
            const StateBuffer* operator*() { return &it->second; }
            const StateBuffer* operator->() { return &it->second; }
        };

        class WriteBuffer {
            friend class Client;
            StateBufferQueue &q;
            HolderMap_t<T> buf;
        public:
            WriteBuffer(StateBufferQueue &q) : q(q) {}
            ~WriteBuffer() { q.add_buffer(std::move(buf)); }
            WriteBuffer(WriteBuffer&&) = default;
            WriteBuffer(const WriteBuffer&) = delete;
            ReadBuffer& operator=(const ReadBuffer&) = delete;
            HolderMap_t<T>* operator->() { return &buf; }
            HolderMap_t<T>* operator*() { return &buf; }
        };

        Client(StateBufferQueue &q) : q(q), id(ids.get_next()) {}

        ReadBuffer get_read_buffer() const {
            return {q, id};
        }
        WriteBuffer get_write_buffer() {
            return {q};
        }
    };
public:
    const Client create_read_only_client() {
        return { *this };
    }

    Client create_client() {
        return { *this };
    }

    StateBufferQueue() { add_buffer({}); }
};

template<template<typename> typename HolderMap_t, template<typename> typename AccessorMap_t, typename T>
utl_prf::IdInc<size_t> StateBufferQueue<HolderMap_t, AccessorMap_t, T>::Client::ids;

