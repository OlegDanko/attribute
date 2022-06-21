#pragma once
//#include "StateBufferQueue/StateBufferQueue.hpp"
#include "StateFrameQueue/StateFrameQueue.hpp"
#include <TypesHelper.hpp>



template<typename, typename>
struct clients_collector;

template<typename, typename>
struct GameStateClient;

template<typename T>
using ReadFrameProvider = typename StateFrameQueue<T>::ReadFrameProvider;

template<typename T>
using ModFrameProvider = typename StateFrameQueue<T>::ModFrameProvider;

template<typename T>
using GenFrameProvider = typename StateFrameQueue<T>::GenFrameProvider;

template<typename T>
using ReadFrame = decltype(ReadFrameProvider<T>::get());

template<typename T>
using ModFrame = decltype(ModFrameProvider<T>::get());

template<typename T>
using GenFrame = decltype(GenFrameProvider<T>::get());
