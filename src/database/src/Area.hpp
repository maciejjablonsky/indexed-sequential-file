#ifndef DATABASE_AREA_HPP
#define DATABASE_AREA_HPP

#include <optional>
#include <utility>
#include <database/Record.hpp>
#include "Link.hpp"
#include <database/Key.hpp>
#include "Entry.hpp"

template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;
namespace area
{
class Area
{
   public:
    optref<area::Entry> FetchEntry(const area::Link link);
    optref<area::Entry> FetchNextEntry(const area::Link link);

};
}  // namespace area
#endif  // DATABASE_AREA_HPP