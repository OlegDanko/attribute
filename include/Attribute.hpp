#pragma once

#include "TypesHelper.hpp"

class Attribute__ {
public:
};

template<typename Base = void, typename Requires = types<>>
class Attribute;

template<typename Base, typename ...Reqs>
class Attribute<Base, types<Reqs...>> : public Base {
public:
    using Base_t = Base;
    using Reqs_t = types<Reqs...>;
};

template<typename ...Reqs>
class Attribute<void, types<Reqs...>> : public Attribute__ {
public:
    using Base_t = void;
    using Reqs_t = types<Reqs...>;
};
