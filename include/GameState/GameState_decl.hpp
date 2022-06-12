#pragma once
#include "StateBufferQueue/StateBufferQueue.hpp"
#include <TypesHelper.hpp>

template<template<typename> typename HolderMap_t, template<typename> typename AccessorMap_t>
struct GS_impl {
    template<typename T>
    struct AttrStateQueue : StateBufferQueue<HolderMap_t, AccessorMap_t, T> {};

    template<typename T>
    using AttrClient = typename AttrStateQueue<T>::Client;

    template<typename T>
    using WriteBuffer = typename AttrClient<T>::WriteBuffer;

    template<typename T>
    using ReadBuffer = typename AttrClient<T>::ReadBuffer;


    template<typename, size_t I = 0>
    struct game_frame_collector;

    template<typename, typename>
    struct clients_collector;

    template<typename, typename>
    struct GameStateFrame;

    template<typename, typename>
    struct GameStateClient;

    template<typename T>
    struct GameState;

};

