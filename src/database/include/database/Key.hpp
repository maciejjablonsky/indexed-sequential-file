#ifndef DATABASE_AREA_KEY_HPP
#define DATABASE_AREA_KEY_HPP

#include <compare>

namespace area
{
struct Key
{
    int value = -1;
    auto operator<=>(const area::Key& lhs) const = default;
};
}  // namespace area

#endif  // DATABASE_AREA_KEY_HPP