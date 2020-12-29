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
    inline decltype(value) min() { return 0; }
    inline decltype(value) max() {
        return std::numeric_limits<decltype(value)>::max();
    }
    inline bool IsDummy() const { return value == dummy(); }
    operator std::string() const { return fmt::format("{}", value); }
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
