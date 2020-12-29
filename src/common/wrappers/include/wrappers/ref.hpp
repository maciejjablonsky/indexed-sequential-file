#pragma once

#include <utility>

namespace wr {
template <typename T> using ref = std::reference_wrapper<T>;
}