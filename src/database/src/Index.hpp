#pragma once

#include "Link.hpp"
#include "Memory.hpp"
#include "PageDispositor.hpp"
#include <algorithm>
#include <concepts/comparable.hpp>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace index {

struct IndexHeader {
    size_t links;
    size_t primary_entries;
    size_t overflow_entries;
};

static_assert(std::is_trivial_v<IndexHeader>);
template <typename T> inline const std::byte *to_byte_ptr(const T *o) {
    return reinterpret_cast<const std::byte *>(o);
}
template <typename T> inline std::byte *to_byte_ptr(T *o) {
    return reinterpret_cast<std::byte *>(o);
}

template <typename Key> concept index_concept = requires { comparable<Key>; };

template <typename Key> requires index_concept<Key> class Index {
  private:
    static constexpr size_t index_entry_size =
        sizeof(Key) + sizeof(link::PrimaryPageLink);
    page::PageDispositor<page::PageMemory<>> page_dispositor_;
    std::map<Key, link::PrimaryPageLink> page_links_;

  public:
    [[nodiscard]] std::tuple<size_t, size_t>
    Setup(const std::string &file_path) {
        if (!page_dispositor_.AttachFile(file_path)) {
            throw std::runtime_error(
                "Failed to setup page_dispositor for database index.");
        }
        if (page_dispositor_.PagesInFile() > 0) {
            return Deserialize(file_path);
        } else {
            key::Key dummy_key = {-1};
            page_links_[dummy_key] = {0};
            return {0, 0};
        }
    }
    [[nodiscard]] link::PrimaryPageLink LookUp(Key key) const {
        auto it = std::adjacent_find(page_links_.begin(), page_links_.end(),
                                     [&](const auto &lhs, const auto &rhs) {
                                         return lhs.first <= key && key < rhs.first;
                                     });
        if (it == page_links_.cend()) {
            --it;
        }
        return it->second;
    }
    inline void Add(Key key, link::PrimaryPageLink link) {
        page_links_.insert_or_assign(key, link);
    }
    inline size_t Size() const { return page_links_.size(); }
    [[nodiscard]] std::tuple<size_t, size_t>
    Deserialize(const std::string &file_path) {
        auto current_page = *page_dispositor_.Request(0);
        IndexHeader header;
        std::copy_n(current_page.begin(), sizeof(header), to_byte_ptr(&header));
        auto entries_read =
            std::min((current_page.size - sizeof(header)) / index_entry_size,
                     header.links);
        auto current_byte = current_page.begin() + sizeof(header);
        auto read_n_entries = [&](size_t n) {
            Key key;
            link::PrimaryPageLink link;
            for (auto i = 0; i < n; ++i) {
                std::copy_n(current_byte, sizeof(key), to_byte_ptr(&key));
                std::copy_n(current_byte + sizeof(Key), sizeof(link),
                            to_byte_ptr(&link));
                current_byte += index_entry_size;
                page_links_[key] = link;
            }
        };
        read_n_entries(entries_read);
        for (auto page_no = 1; entries_read < header.links; ++page_no) {
            current_page = *page_dispositor_.Request(page_no);
            current_byte = current_page.begin();
            auto entries_to_read =
                std::min(current_page.size / index_entry_size,
                         header.links - entries_read);
            read_n_entries(entries_to_read);
            entries_read += entries_to_read;
        }

        return {header.primary_entries, header.overflow_entries};
    }
    void Serialize(size_t primary_entries, size_t overflow_entries) {
        page_dispositor_.ClearFile();
        IndexHeader header = {.links = page_links_.size(),
                              .primary_entries = primary_entries,
                              .overflow_entries = overflow_entries};

        auto current_page = *page_dispositor_.Request(0);
        std::copy_n(to_byte_ptr(&header), sizeof(header),
                    current_page.begin()); // copy header

        auto entries_saved =
            (current_page.size - sizeof(header)) / index_entry_size;
        auto current_byte = current_page.begin() + sizeof(header);
        auto page_link_it = page_links_.begin();
        // prepare first page
        std::for_each_n(
            page_links_.begin(), std::min(page_links_.size(), entries_saved),
            [&](const auto &index_entry) {
                current_byte =
                    std::copy_n(to_byte_ptr(&index_entry.first),
                                sizeof(index_entry.first), current_byte);
                current_byte =
                    std::copy_n(to_byte_ptr(&index_entry.second),
                                sizeof(index_entry.second), current_byte);
                ++page_link_it;
            });
        page_dispositor_.Write(current_page);

        for (auto page_no = 1; entries_saved < page_links_.size(); ++page_no) {
            auto current_page = *page_dispositor_.Request(page_no);
            current_byte = current_page.begin();
            auto entries_to_save =
                std::min(current_page.size / index_entry_size,
                         page_links_.size() - entries_saved);
            std::for_each_n(
                page_link_it, entries_to_save, [&](const auto &index_entry) {
                    current_byte =
                        std::copy_n(to_byte_ptr(&index_entry.first),
                                    sizeof(index_entry.first), current_byte);
                    current_byte =
                        std::copy_n(to_byte_ptr(&index_entry.second),
                                    sizeof(index_entry.second), current_byte);
                    ++page_link_it;
                });
            page_dispositor_.Write(current_page);
            entries_saved += entries_to_save;
        }
    }
    void Show() {
        fmt::print("[{:^91}]\n", "INDEX");
        for (const auto &[key, link] : page_links_) {
            fmt::print("[key: {:>4}, page: {:>4}]\n",
                       static_cast<std::string>(key), link);
        }
        fmt::print("\n");
    }
    void Clear() {
        page_links_.clear();
        page_dispositor_.ClearFile();
    }
};
} // namespace index
