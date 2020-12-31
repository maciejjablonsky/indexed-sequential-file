#pragma once

#include "DiskAccess.hpp"
#include "Link.hpp"
#include "Memory.hpp"
#include "PageDispositor.hpp"
#include "PageWithEntries.hpp"
#include <overloaded/overloaded.hpp>
#include <type_traits>
#include <utility>
#include <variant>
#include <wrappers/optref.hpp>

namespace area {

template <typename Entry> class Area {
  private:
    using clean_entries_page =
        page::CleanPageWithEntries<page::PageMemory<>, Entry>;
    using dirty_entries_page =
        page::DirtyPageWithEntries<page::PageMemory<>, Entry>;
    using entries_page = std::variant<clean_entries_page, dirty_entries_page>;

    mutable page::PageDispositor<page::PageMemory<>> page_dispositor_;
    mutable std::optional<entries_page> loaded_page_;
    mutable std::optional<entries_page> buffered_page_;
    size_t stored_entries_ = 0;

  private:
    void MoveLoadedToBuffer() const {
        if (buffered_page_ &&
            std::holds_alternative<dirty_entries_page>(*buffered_page_)) {
            buffered_page_ = clean_entries_page(
                std::get<dirty_entries_page>(*buffered_page_).Release());
            page_dispositor_.Write(
                std::get<clean_entries_page>(*buffered_page_).Memory());
        }
        buffered_page_ = std::move(loaded_page_);
    }

  protected:
    wr::optional_ref<entries_page> LoadPage(size_t page_no) const {
        auto is_page_loaded = [&](const entries_page &var_page) {
            return std::visit(overloaded{[&](const auto &page) {
                                  return page.Index() == page_no;
                              }},
                              var_page);
        };
        if (loaded_page_ && is_page_loaded(*loaded_page_)) {
            return *loaded_page_;
        } else if (buffered_page_ && is_page_loaded(*buffered_page_)) {
            std::swap(loaded_page_, buffered_page_);
            return *loaded_page_;
        } else {
            MoveLoadedToBuffer();
            auto opt_page = page_dispositor_.Request(page_no);
            if (opt_page) {
                loaded_page_ = clean_entries_page(std::move(*opt_page));
                return *loaded_page_;
            } else {
                loaded_page_ = std::nullopt;
                return std::nullopt;
            }
        }
    }
    std::optional<link::EntryLink> AppendToPage(const Entry &entry,
                                                link::PageLink link) {
        auto opt_var_page = LoadPage(link);
        if (opt_var_page) {
            auto &var_page = wr::get_ref<entries_page>(opt_var_page);
            if (!std::visit(overloaded{[&](const auto &page) {
                                return page.Size() < page.Capacity();
                            }},
                            var_page)) {
                return std::nullopt;
            }

            if (std::holds_alternative<clean_entries_page>(var_page)) {
                var_page = dirty_entries_page(
                    std::move(std::get<clean_entries_page>(var_page)));
            }
            auto &dirty_page = std::get<dirty_entries_page>(var_page);
            link::EntryLink link_to_inserted = {
                .page = link,
                .entry = static_cast<int32_t>(dirty_page.Append(entry))};
            ++stored_entries_;
            return link_to_inserted;
        }
        return std::nullopt;
    }

    link::EntryLink PushBack(const Entry &entry) {
        link::PageLink last_page_link = {
            static_cast<int32_t>(page_dispositor_.PagesInFile() - 1)};
        if (auto inserted_link = AppendToPage(entry, last_page_link)) {
            return *inserted_link;
        } else {
            return *AppendToPage(entry, last_page_link + 1);
        }
    }

  public:
    inline void ResetDiskAccessCounter() { page_dispositor_.ResetCounter(); }
    link::EntryLink IncrementLinkToExistingEntry(link::EntryLink link) {
        link::EntryLink area_link = link;
        if (link.entry < LoadedPageSize() - 1) {
            area_link.entry += 1;
        } else {
            area_link.page += 1;
            area_link.entry = 0;
        }
        return area_link;
    }

    std::pair<wr::optional_ref<const Entry>, link::EntryLink>
    ViewSubsequent(const link::EntryLink &link) const {
        link::EntryLink next_entry = {.page = link.page,
                                      .entry = link.entry + 1};
        if (auto opt_ref_entry = View(next_entry)) {
            return {opt_ref_entry, next_entry};
        }
        link::EntryLink next_page_entry = {.page = link.page + 1, .entry = 0};
        if (auto opt_ref_entry = View(next_page_entry)) {
            return {opt_ref_entry, next_page_entry};
        }
        return {std::nullopt, link};
    }

    wr::optional_ref<const Entry> View(link::EntryLink link) const {
        if (link.page >= page_dispositor_.PagesInFile()) {
            return std::nullopt;
        }
        if (auto opt_var_page = LoadPage(link.page)) {
            return std::visit(
                overloaded{
                    [&](const auto &page) -> wr::optional_ref<const Entry> {
                        if (link.entry >= page.Size()) {
                            return std::nullopt;
                        }
                        return page.View(link.entry);
                    }},
                opt_var_page->get());
        }
        return std::nullopt;
    }

    inline void Insert(const Entry &entry, link::EntryLink destination) {
        auto opt_var_page = LoadPage(destination.page);
        if (!opt_var_page) {
            throw std::runtime_error(
                fmt::format("Failed to load page no {}\n", destination.page));
        }
        auto &var_page = wr::get_ref<entries_page>(opt_var_page);
        if (std::holds_alternative<clean_entries_page>(var_page)) {
            var_page = dirty_entries_page(
                std::move(std::get<clean_entries_page>(var_page)));
        }

        auto &dirty_page = std::get<dirty_entries_page>(var_page);
        dirty_page.Write(entry, destination.entry);
        if (destination.entry == dirty_page.Size()) {
            ++stored_entries_;
        }
    }

    inline void Setup(const std::string &file_path, size_t stored_entries = 0) {
        page_dispositor_.AttachFile(file_path);
        stored_entries_ = stored_entries;
    };
    inline size_t Size() const { return stored_entries_; }
    inline void Save() const {
        MoveLoadedToBuffer(); // saves buffered page if needed and moves
                              // loaded to buffered
        MoveLoadedToBuffer(); // saves loaded as buffered page and moves
                              // nullopt to buffer
        loaded_page_ = buffered_page_ = std::nullopt;
    };

    void Show() {
        for (auto i = 0; i < page_dispositor_.PagesInFile(); ++i) {
            auto opt_page = LoadPage(i);
            if (opt_page) {
                std::visit(overloaded{[](const auto &page) { page.Show(); }},
                           wr::get_ref<entries_page>(opt_page));
            }
        }
        fmt::print("\n");
    }

    inline DiskAccess GetDiskAccesses() const {
        return page_dispositor_.GetDiskAccessCounter();
    }

    inline void SetDiskAccesses(const DiskAccess &counter) {
        page_dispositor_.SetDiskAccessCounter(counter);
    }

    void Clear() {
        loaded_page_ = buffered_page_ = std::nullopt;
        page_dispositor_.ClearFile();
        stored_entries_ = 0;
    }

    void RenameAndSwapFile(const std::string &from, const std::string &to,
                           DiskAccess external_counter = {0, 0},
                           size_t size = 0) {
        Save();
        page_dispositor_.CloseFile();
        std::filesystem::remove(to);
        std::filesystem::rename(from, to);
        page_dispositor_.AttachFile(to);
        SetDiskAccesses(GetDiskAccesses() + external_counter);
        stored_entries_ = size;
    }

    inline size_t SinglePageCapacity() const {
        return clean_entries_page::Capacity();
    }

    inline size_t LoadedPageSize() const {
        if (loaded_page_) {
            return std::visit(
                overloaded{[&](const auto &page) { return page.Size(); }},
                *loaded_page_);
        }
        return 0;
    }
};
} // namespace area