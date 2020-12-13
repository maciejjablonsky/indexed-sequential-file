#ifndef DATABASE_AREA_KEY_HPP
#define DATABASE_AREA_KEY_HPP

#include <compare>
#include <sstream>
#include <string>

namespace area
{
struct Key
{
    int32_t value = -1;
    auto operator<=>(const area::Key& lhs) const = default;
    friend std::stringstream& operator>>(std::stringstream& ss, area::Key& key);
    operator std::string() const;
};
}  // namespace area

#endif  // DATABASE_AREA_KEY_HPP