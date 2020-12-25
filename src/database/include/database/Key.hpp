#pragma once

#include <compare>
#include <sstream>
#include <string>
#include <numeric>
#include <variant>
#include <type_traits>
#include <fmt/format.h>
#include <overloaded/overloaded.hpp>

namespace key
{
struct DummyKey
{
    int32_t value = -1;
};

struct ActiveKey
{
    int32_t value;
    inline auto min() { return 0; }
    inline auto max() { return std::numeric_limits<decltype(value)>::max(); }
    auto operator<=>(const ActiveKey& rhs) const = default;
    inline operator std::string() { return fmt::format("{}", value); }
};

struct IndexKey
{
    int32_t value;
    auto operator<=>(const IndexKey& other) const = default;
    inline auto min() { return -1; }
    inline auto max() { return std::numeric_limits<decltype(value)>::max(); }
};

using Key = std::variant<DummyKey, ActiveKey>;

inline std::stringstream& operator<<(std::stringstream& ss, const Key& entry_key)
{
    std::visit(overloaded{[&](const ActiveKey& key) { ss << key.value; },
                          [&](const DummyKey& dummy_key) { ss << dummy_key.value; }},
               entry_key);
    return ss;
}

inline std::stringstream& operator>>(std::stringstream& ss, ActiveKey& key)
{
    ss >> key.value;
    return ss;
}

}  // namespace key
