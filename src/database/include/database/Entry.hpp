#ifndef DATABASE_AREA_ENTRY_HPP
#define DATABASE_AREA_ENTRY_HPP

#include <database/Link.hpp>
#include <database/Record.hpp>
#include <database/Key.hpp>
#include <compare>

namespace area {
struct Entry
{
    area::Key key;
    area::Record record;
    area::Link link;
    bool deleted = false;
    auto operator<=>(const Entry& other) const;
};
}

#endif // DATABASE_AREA_ENTRY_HPP