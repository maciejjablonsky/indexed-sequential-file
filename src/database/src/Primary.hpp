#pragma once

#include "Area.hpp"
#include "Link.hpp"
#include <concepts/comparable.hpp>
#include <database/Key.hpp>
#include <overloaded/overloaded.hpp>

namespace primary {
template <typename Entry> concept primary_concept = requires(Entry entry) {
    comparable<Entry>,
        std::is_same_v<decltype(entry.PointsTo()), link::EntryLink>,
        std::is_convertible_v<Entry, bool>;
};

struct EntryAlreadyInPrimary {};

struct EntryInserted {};

struct EntryToOverflow {
    link::PrimaryEntryLink from_primary_start_link;
};

struct EntryToPrimary {};
struct EntryInsertedOnNewPage {
    link::PrimaryPageLink link;
};

template <typename Entry>
requires primary_concept<Entry> class Primary : public area::Area<Entry> {

  public:
    std::variant<EntryInserted, EntryInsertedOnNewPage>
    Append(const Entry &new_entry, link::PrimaryPageLink page_link) {
        if (!AppendToPage(new_entry, page_link)) {
            auto next_page_link = page_link + 1;
            if (AppendToPage(new_entry, next_page_link)) {
                return EntryInsertedOnNewPage{next_page_link};
            } else {
                throw std::runtime_error(fmt::format(
                    "Failed to append entry to primary area on {} page.\n",
                    page_link));
            }
        }
        return EntryInserted{};
    }

    std::variant<EntryToOverflow, EntryAlreadyInPrimary, EntryToPrimary>
    LookThrough(const Entry &new_entry, link::PrimaryPageLink page_link) {
        link::EntryLink entry_link = {.page = page_link, .entry = 0};
        auto opt_entry = View(entry_link);
        link::EntryLink prev_link = entry_link;
        while (opt_entry) {
            const auto &entry = wr::get_ref<const Entry>(opt_entry);
            if (!entry.IsDeleted() && (entry > new_entry)) {
                return EntryToOverflow{.from_primary_start_link = prev_link};
            } else if (!entry.IsDeleted() && (entry == new_entry)) {
                return EntryAlreadyInPrimary{};
            }
            prev_link = entry_link;
            auto [opt_entry_, entry_link_] = ViewSubsequent(entry_link);
            opt_entry = std::move(opt_entry_);
            entry_link = std::move(entry_link_);
        }
        return EntryToPrimary{};
    }
};
} // namespace primary
