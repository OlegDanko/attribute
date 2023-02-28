#pragma once

#include <utils/types/Types.hpp>

template<typename...>
struct attr_base_collision;

template<typename Attr1>
struct attr_base_collision<Attr1> {
    static constexpr bool check() { return true; };
};

template<typename Attr1, typename Attr2, typename ...Attrs>
struct attr_base_collision<Attr1, Attr2, Attrs...> {
    static constexpr bool check() {
        constexpr bool is_same = std::is_same<typename Attr1::Base_t, typename Attr2::Base_t>::value;
        constexpr bool is_void = std::is_same<typename Attr1::Base_t, void>::value;
        static_assert(!is_same || is_void, "Base collision detected: two different attributes with of same base attribute requested");
        return (!is_same || is_void) && attr_base_collision<Attr1, Attrs...>::check() && attr_base_collision<Attr2, Attrs...>::check();
    }
};


template<typename...Attrs>
struct attr_base_collision<types<Attrs...>> {
    static constexpr bool check() {
       return attr_base_collision<Attrs...>::check();
    }
};

template<typename...>
struct check_attr_base_collision;

template<typename...>
struct attributes_list;

template<typename ...Attrs>
struct attributes_list<types<Attrs...>> {
    using types_ = typename attributes_list<Attrs...>::types_;
};

template<>
struct attributes_list<> {
    using types_ = types<>;
};

template<typename Attr, typename ...Attrs>
class attributes_list<Attr, Attrs...> {
    using attr_types = typename attributes_list<Attr>::types_;
    using reqs_types = typename attributes_list<Attrs...>::types_;
    using all_types = typename cat_types<attr_types, reqs_types>::types_;

public:
    using types_ = typename unique_types<all_types>::types_;
    static_assert(attr_base_collision<types_>::check());
};

template<typename Attr>
class attributes_list<Attr> {
    using reqs_types = typename attributes_list<typename Attr::Reqs_t>::types_;
    using all_types = typename cat_types<Attr, reqs_types>::types_;
public:
    using types_ = typename unique_types<all_types>::types_;
    static_assert(attr_base_collision<types_>::check());
};
