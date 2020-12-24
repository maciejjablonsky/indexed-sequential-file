#pragma once

#include <vector>
#include <utility>
#include "PageDispositor.hpp"
#include "Memory.hpp"
#include <concepts/comparable.hpp>
#include <map>
#include <type_traits>
#include <string>

namespace index
{

struct IndexHeader
{
    size_t links;
    size_t primary_entries;
    size_t overflow_entries;
};

template <typename Key, typename PageLink>
concept index_concept = requires
{
    comparable<Key>, comparable<PageLink>, std::is_integral_v<PageLink>;
};

template <typename Key, typename PageLink>
requires index_concept<Key, PageLink> class Index
{
   public:
    [[nodiscard]] bool Setup(const std::string& file_path);
    [[nodiscard]] PageLink LookUp(const Key key) const;
    void Add(Key key, PageLink link);
    inline size_t Size() const { return page_links_.size(); }
    [[nodiscard]] std::pair<size_t, size_t> Deserialize(const std::string& path);
    void Serialize(size_t primary_entries, size_t overflow_entries);

   private:
    page::PageDispositor<page::PageMemory<>> page_dispositor_;
    std::map<Key, PageLink> page_links_;
};
template <typename Key, typename PageLink>
requires index_concept<Key, PageLink> inline bool Index<Key, PageLink>::Setup(const std::string& file_path)
{
    return page_dispositor_.Setup(file_path);
}
template <typename Key, typename PageLink>
requires index_concept<Key, PageLink> inline PageLink Index<Key, PageLink>::LookUp(const Key key) const
{
    auto it = page_links_.lower_bound(key);
    if (it == page_links_.cend())
    {
        --it;
    }
    return it->second;
}
template <typename Key, typename PageLink>
requires index_concept<Key, PageLink> inline void Index<Key, PageLink>::Add(Key key, PageLink link)
{
    ++header_.links;
    page_links_.insert_or_assign(key, link);
}

template <typename Key, typename PageLink>
requires index_concept<Key, PageLink> inline void Index<Key, PageLink>::Serialize(size_t primary_entries,
                                                                                  size_t overflow_entries)
{
    IndexHeader header = {
        .links = page_links_.size(), .primary_entries = primary_entries, .overflow_entries = overflow_entries};
    size_t serialized_page = 0;
    size_t serialized_entries = 0;
    page_dispositor_.Reset();
    auto current_page = page_dispositor_.Request(serialized_page);
    ++serialized_page;
}
}  // namespace index
