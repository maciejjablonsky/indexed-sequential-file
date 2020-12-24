#pragma once

#include <concepts>

template <typename T>
concept comparable = requires(T a, T b)
{
    a<b, a == b, a> b, a != b;
};

template <typename T, typename U>
concept comarable_with = requires(T t, U u)
{
    t<u, t == u, t> u, t != u, u<t, u == t, u> t, u != t;
};