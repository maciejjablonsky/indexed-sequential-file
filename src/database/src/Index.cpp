#include "Index.hpp"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <algorithm>

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

void db::Index::Deserialize(const std::string &path, int page_size)
{
    if ((std::filesystem::exists(path) &&
         std::filesystem::file_size(path) == 0) ||
        !std::filesystem::exists(path))
    {

        Reset(page_size);
    }
    else
    {
        Load(path, page_size);
    }
}

void db::Index::Load(const std::string &path, int page_size)
{
    std::vector<std::byte> page(page_size);
    std::ifstream ifs(path, std::ios::binary);
    ifs.read(reinterpret_cast<char *>(page.data()), page_size);
    if (ifs.eof())
    {
        throw std::runtime_error(
            "Attempted to load index file, but it's empty.");
    }
    header_ = *reinterpret_cast<IndexHeader *>(page.data());
    auto page_header =
        *reinterpret_cast<IndexPageHeader *>(page.data() + sizeof(IndexHeader));
    for (int i = 0; i < page_header.entries_number; ++i)
    {
        lookup_table_.push_back(*reinterpret_cast<key_link *>(
            page.data() + sizeof(IndexHeader) + sizeof(IndexPageHeader) +
            i * sizeof(key_link)));
    }

    for (int page_no = 1; page_no < header_.index_pages; ++page_no)
    {
        ifs.read(reinterpret_cast<char *>(page.data()), page_size);
        page_header = *reinterpret_cast<IndexPageHeader *>(page.data());
        for (int i = 0; i < page_header.entries_number; ++i)
        {
            lookup_table_.push_back(*reinterpret_cast<key_link *>(
                page.data() + sizeof(IndexPageHeader) + i * sizeof(key_link))); 
        }
    }
}

void db::Index::Reset(int page_size)
{
    header_ = {.primary_pages = 1, .index_pages = 1, .page_size = page_size};
    lookup_table_.clear();
    lookup_table_.emplace_back(area::Key{-1}, area::Link{0, 0});
}
