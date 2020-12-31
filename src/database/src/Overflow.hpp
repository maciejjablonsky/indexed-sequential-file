#pragma once
#include "Area.hpp"
#include <string>
#include <wrappers/optref.hpp>

namespace overflow {
struct EntryAlreadyInOverflow {
    link::OverflowEntryLink link;
};

struct AppendWithoutPointing {
    link::OverflowEntryLink update;
};

struct AppendWithPointing {
    link::OverflowEntryLink point;
    link::OverflowEntryLink update;
};

struct FirstWasLarger {
    link::OverflowEntryLink point;
};

struct EntryNotFound {};

struct EmptyOverflow {};
template <typename Entry> concept overflow_concept = requires(Entry entry) {

    comparable<Entry>,
        std::is_same_v<decltype(entry.PointsTo()), link::EntryLink>,
        std::is_same_v<decltype(entry.IsDeleted()), bool>;
};

template <typename Entry>
requires overflow_concept<Entry> class Overflow : public area::Area<Entry> {
  public:
    std::variant<EntryAlreadyInOverflow, AppendWithoutPointing,
                 AppendWithPointing, FirstWasLarger, EmptyOverflow>
    LookThrough(const Entry &new_entry, link::EntryLink link) {
        link::EntryLink prev_link = link;
        auto opt_entry = View(link);
        if (!opt_entry) {
            return EmptyOverflow{};
        }
        if (wr::get_ref<const Entry>(opt_entry) > new_entry) {
            return FirstWasLarger{.point = link};
        }
        while (opt_entry) {
            const auto &entry = wr::get_ref<const Entry>(opt_entry);
            if (!entry.IsDeleted() && entry > new_entry) {
                return AppendWithPointing{.point = link, .update = prev_link};

            } else if (!entry.IsDeleted() && entry == new_entry) {
                return EntryAlreadyInOverflow{};
            }

            prev_link = link;
            if (auto opt_link = entry.PointsTo()) {
                link = *opt_link;
            } else {
                break;
            }
            opt_entry = View(link);
        }
        return AppendWithoutPointing{.update = prev_link};
    }

    link::OverflowEntryLink Append(const Entry &entry) {
        auto link = PushBack(entry);
        return {link.page, link.entry};
    }

    template <typename Key>
    std::variant<EntryAlreadyInOverflow, EntryNotFound>
    LookThrough(Key key, link::EntryLink entry_link) {
        auto opt_entry = View(entry_link);
        link::EntryLink prev_link = entry_link;
        while (opt_entry) {
            const auto &entry = wr::get_ref<const Entry>(opt_entry);
            if (!entry.IsDeleted() && (entry > key)) {
                return EntryNotFound{};
            } else if (!entry.IsDeleted() && (entry == key)) {
                return EntryAlreadyInOverflow{.link = entry_link};
            }
            prev_link = entry_link;
            if (auto opt_link = entry.PointsTo()) {
                entry_link = *opt_link; 
            } else {
                break; 
            }

            opt_entry = View(entry_link);
        }
        return EntryNotFound{};
 
    }
};
} // namespace overflow