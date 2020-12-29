#pragma once

#include <wrappers/opt.hpp>
#include <wrappers/ref.hpp>

namespace wr {
template <typename T> using optional_ref = wr::opt<wr::ref<T>>;

template <typename T> T &get_ref(optional_ref<T> &optref) {
    return optref->get();
}
template <typename T> const T &get_ref(const optional_ref<T> &optref) {
    return optref->get();
}
} // namespace wr