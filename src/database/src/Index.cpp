#include "Index.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>

area::Link db::Index::LookUp(const area::Key key) const
{
    auto it = std::adjacent_find(lookup_table_.cbegin(), lookup_table_.cend(),
                                 [&](const auto &lhs, const auto &rhs) {
                                     return lhs.key <= key && key < rhs.key;
                                 });
    if (it == lookup_table_.cend())
    {
        return lookup_table_.back().link;
    }
    return it->link;
}

void db::Index::Add(const area::Key key, const area::Link link)
{
    lookup_table_.push_back({key, link});
}

void db::Index::Deserialize(const std::string &path)
{
    if ((std::filesystem::exists(path) &&
         std::filesystem::file_size(path) == 0) ||
        !std::filesystem::exists(path))
    {
        lookup_table_.push_back({area::Key{-1}, area::Link{0, 0}});
    }
    else
    {
        auto filesize = std::filesystem::file_size(path);
        std::ifstream ifs(path, std::ios::binary);
        auto entries_num = filesize / (sizeof(db::key_link));
        lookup_table_.reserve(entries_num);
        ifs.read(reinterpret_cast<char *>(lookup_table_.data()), filesize);
    }
}
