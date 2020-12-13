#include "Entry.hpp" 

auto area::Entry::operator<=>(const Entry& other) const
{
    auto ret = (key == other.key);
    if (ret)
    {
        return deleted == other.deleted;
    }
    return ret;
}

