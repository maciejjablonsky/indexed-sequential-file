#include "Index.hpp"
#include <algorithm>

area::Link db::Index::LookUp(const area::Key key) const
{
    auto it = std::adjacent_find(lookup_table_.cbegin(), lookup_table_.cend(),
                                 [&](const auto &lhs, const auto &rhs) {
                                     return lhs.first <= key && key < rhs.first;
                                 });
    if (it == lookup_table_.cend())
    {
        return lookup_table_.back().second;
    }
    return it->second;
}

void db::Index::Add(const area::Key key, const area::Link link)
{
    lookup_table_.emplace_back(key, link);
}