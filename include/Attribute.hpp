#pragma once

#include <utils/types/Types.hpp>

using utl_prf::types;

class Attribute__ {
protected:
    std::size_t id;
public:
    Attribute__(std::size_t id) : id(id) {}
    std::size_t get_id() const { return id; }
};

template<typename Base = void, typename Requires = types<>>
class Attribute;

template<typename ...Reqs>
class Attribute<void, types<Reqs...>> : public Attribute__ {
public:
    using Base_t = void;
    using Reqs_t = types<Reqs...>;
    Attribute(std::size_t id) : Attribute__(id) {}
    Attribute(const Attribute& that) : Attribute__(that.id) {}
    Attribute(Attribute&& that) : Attribute__(that.id) {}
};

template<typename Base, typename ...Reqs>
class Attribute<Base, types<Reqs...>> : public Base {
public:
    using Base_t = Base;
    using Reqs_t = types<Reqs...>;
    Attribute(std::size_t id) : Base(id) {}
    Attribute(const Attribute& that) : Base(that.id) {}
    Attribute(Attribute&& that) : Base(that.id) {}
};
