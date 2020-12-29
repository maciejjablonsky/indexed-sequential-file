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
            return *buffered_page_;
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
    std::pair<wr::optional_ref<const Entry>, link::EntryLink>
    ViewSubsequent(link::EntryLink &link) const {
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
        page_dispositor_.Setup(file_path);
        stored_entries_ = stored_entries;
    };
    inline size_t Size() const { return stored_entries_; }
    inline void Save() const {
        MoveLoadedToBuffer(); // saves buffered page if needed and moves
                              // loaded to buffered
        MoveLoadedToBuffer(); // saves loaded as buffered page and moves
                              // nullopt to buffer
    };

    void Show() {
        link::EntryLink link = {0, 0};
        auto opt_entry = View(link);
        if (!opt_entry) {
            return;
        }
        do {
            fmt::print("location: {}, entry: {}\n",
                       static_cast<std::string>(link),
                       static_cast<std::string>(wr::get_ref<const Entry>(opt_entry)));
            auto [opt_entry_, link_] = ViewSubsequent(link);
            opt_entry = std::move(opt_entry_);
            link = std::move(link_);
        } while (opt_entry);
        fmt::print("\n");
    }
};
} // namespace area