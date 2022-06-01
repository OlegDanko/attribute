#pragma once
#include <type_traits>

template<typename ... Ts>
struct types;

template<typename, typename>
struct cat_types;
template<typename T, typename ...Ts>
struct cat_types<T, types<Ts...>> {
    using types_ = types<T, Ts...>;
};
template<typename ...Ts1, typename ...Ts2>
struct cat_types<types<Ts1...>, types<Ts2...>> {
    using types_ = types<Ts1..., Ts2...>;
};

template<typename, typename...>
struct is_unique;

template<typename T1, typename T2>
struct is_unique<T1, T2> { static constexpr bool val = (!std::is_same<T1, T2>::value); };

template<typename T1, typename T2, typename ...Ts>
struct is_unique<T1, T2, Ts...> { static constexpr bool val = (!std::is_same<T1, T2>::value) && is_unique<T1, Ts...>::val; };


template<typename ... Ts>
struct unique_types;

template<typename T>
struct unique_types<T> {
    using types_ = types<T>;
};

template<typename T, typename ...Ts>
class unique_types<T, Ts...> {
    using tail_types_ = typename unique_types<Ts...>::types_;
    using all_types = typename cat_types<T, tail_types_>::types_;
public:
    using types_ = typename std::conditional<is_unique<T, Ts...>::val,
                                            all_types,
                                            tail_types_>::type;
};

template<typename ...Ts>
struct unique_types<types<Ts...>> {
    using types_ = typename unique_types<Ts...>::types_;
};
