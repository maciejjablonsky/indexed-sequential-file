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

void db::Index::AddPrimaryPage(const area::Key key, const area::Link link)
{
    lookup_table_.push_back({key, link});
}

bool db::Index::Load(const std::string &path, int page_size)
{
    if (!Deserialize(path, page_size))
    {

        Reset(page_size);
    }
    return true;
}

bool db::Index::Deserialize(const std::string &path, int page_size)
{
    std::vector<std::byte> page(page_size);
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open())
    {
        return false;
    }
    ifs.read(reinterpret_cast<char *>(page.data()), page_size);
    if (ifs.eof())
    {
        return false;
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

    while (!ifs.eof())
    {
        ifs.read(reinterpret_cast<char *>(page.data()), page_size);
        page_header = *reinterpret_cast<IndexPageHeader *>(page.data());
        for (int i = 0; i < page_header.entries_number; ++i)
        {
            lookup_table_.push_back(*reinterpret_cast<key_link *>(
                page.data() + sizeof(IndexPageHeader) + i * sizeof(key_link)));
        }
    }
    return true;
}

void db::Index::Increment(counter which)
{
    switch (which)
    {
        case counter::primary_pages:
            ++header_.primary_pages;
            break;
        case counter::primary_entries:
            ++header_.primary_entries;
            break;
        case counter::overflow_pages:
            ++header_.overflow_pages;
            break;
        case counter::overflow_entries:
            ++header_.overflow_entries;
            break;
        default:
            break;
    }
}

void db::Index::Decrement(counter which)
{
    switch (which)
    {
        case counter::primary_pages:
            --header_.primary_pages;
            break;
        case counter::primary_entries:
            --header_.primary_entries;
            break;
        case counter::overflow_pages:
            --header_.overflow_pages;
            break;
        case counter::overflow_entries:
            --header_.overflow_entries;
            break;
        default:
            break;
    }
}

bool db::Index::IsLastPage(area::Link link) { return link.page_no == header_.primary_pages - 1; }

void db::Index::Reset(int page_size)
{
    header_ = {.primary_pages = 1, .page_size = page_size};
    lookup_table_.clear();
    lookup_table_.push_back({area::Key{-1}, area::Link{0, 0}});
}
