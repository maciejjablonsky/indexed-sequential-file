#ifndef DATABASE_AREA_LINK_HPP
#define DATABASE_AREA_LINK_HPP

#include <cstdint>

namespace area
{
struct Link
{
    int32_t page_no;
    int32_t entry_index;
    Link() : page_no(-1), entry_index(-1) {}
};
}  // namespace area

#endif  // DATABASE_AREA_LINK_HPP