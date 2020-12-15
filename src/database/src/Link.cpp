#include "Link.hpp"

bool area::Link::IsActive() const
{
    return page_no > -1 && entry_index > -1;
}
