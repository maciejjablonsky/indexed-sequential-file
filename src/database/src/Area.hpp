#ifndef DATABASE_AREA_HPP
#define DATABASE_AREA_HPP

#include <variant>
#include <optional>
#include <utility>
#include <database/Record.hpp>
#include "Link.hpp"

template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;

namespace area
{
using record_or_link =
    std::variant<optref<const area::Record>, const area::Link>;
class Area
{
   public:
    /**
     * @brief If record doesn't exist or is inside primary area then
     * optref<Record> is returned, else if in primary area there is link to
     * overflow, then this link is returned so it can be used to lookup
     * overflow area.
     */
    record_or_link FetchRecord(const area::Link link) const;
};
}  // namespace area
#endif  // DATABASE_AREA_HPP