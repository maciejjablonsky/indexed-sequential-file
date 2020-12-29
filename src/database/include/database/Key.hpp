#pragma once

#include <compare>
#include <fmt/format.h>
#include <numeric>
#include <overloaded/overloaded.hpp>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

namespace key {

struct Key {
    int32_t value;
    auto operator<=>(const Key &other) const = default;
    inline decltype(value) dummy() const { return -1; }
    static inline decltype(value) min() { return 0; }
    static inline decltype(value) max() {
        return std::numeric_limits<decltype(value)>::max();
    }
    inline bool IsDummy() const { return value == dummy(); }
    operator std::string() const { return fmt::format("{:>6}", value); }
    static bool IsValid(const Key &key) {
        return Key::min() <= key.value && key.value <= Key::max();
    }
    bool IsValid() const { return Key::IsValid(*this); }
};

static_assert(std::is_trivial_v<Key>);

inline std::stringstream &operator>>(std::stringstream &ss, Key &key) {
    ss >> key.value;
    return ss;
}

inline std::stringstream &operator<<(std::stringstream &ss, Key &key) {
    ss << key.value;
    return ss;
}

} // namespace key
