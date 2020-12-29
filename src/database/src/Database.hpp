#pragma once

#include "Index.hpp"
#include "Link.hpp"
#include "Overflow.hpp"
#include "Primary.hpp"
#include <compare>
#include <concepts/comparable.hpp>
#include <concepts>
#include <database/Key.hpp>
#include <database/Record.hpp>
#include <optional>
#include <type_traits>
#include <utility>
#include <wrappers/optref.hpp>

template <typename T> using optref = std::optional<std::reference_wrapper<T>>;
namespace db {

template <typename Record> concept database_concept = requires {
    std::is_trivial_v<Record>;
};

template <typename Record> struct Entry {
    key::Key key;
    Record record;
    link::EntryLink link;
    bool deleted;
    inline std::optional<link::EntryLink> PointsTo() const {
        if (link) {
            return link;
        }
        return std::nullopt;
    }
    std::strong_ordering operator<=>(const Entry &other) const {
        return key <=> other.key;
    }
    auto operator==(const Entry &other) const { return key == other.key; }
    // inline bool operator==(const Entry &other) const {
    //    return key == other.key;
    //}
    static Entry dummy() {
        Entry dummy = {.key = {-1}, .link = {-1, -1}};
        memset(&dummy.record, 0xaa, sizeof(Record));
        return dummy;
    }
    operator std::string() const {
        return fmt::format("{{key: {}, content: {}, pointing_to: {}}}",
                           static_cast<std::string>(key),
                           static_cast<std::string>(record),
                           static_cast<std::string>(link));
    }
    Entry &operator=(const link::EntryLink &link) {
        this->link = link;
        return *this;
    }
    Entry &operator=(const link::OverflowEntryLink &overflow_link) {
        link.entry = overflow_link.entry;
        link.page = overflow_link.page;
        return *this;
    }
    Entry &operator=(const link::PrimaryEntryLink &primary_link) {
        link.entry = primary_link.entry;
        link.page = primary_link.page;
        return *this;
    }
    inline bool IsDeleted() const { return deleted; }
};

template <typename Record> requires database_concept<Record> class DataBase {
  private:
    float autoreorganization_ = 0.2;
    float page_utilization_ = 0.5;
    index::Index<key::Key> index_;
    primary::Primary<Entry<Record>> primary_;
    overflow::Overflow<Entry<Record>> overflow_;

  private:
    void AppendToPrimary(const Entry<Record> &entry,
                         link::PrimaryPageLink page_link) {
        std::visit(
            overloaded{[&](const primary::EntryInsertedOnNewPage &message) {
                           index_.Add(entry.key, message.link);
                       },
                       [&](const primary::EntryInserted &) {}},
            primary_.Append(entry, page_link));
        fmt::print("Inserted to primary: {}\n",
                   static_cast<std::string>(entry));
    }

    void AddToOverflow(Entry<Record> &entry,
                       const link::PrimaryEntryLink &primary_link) {
        using namespace overflow;
        auto primary_entry =
            wr::get_ref<const Entry<Record>>(primary_.View(primary_link));
        if (auto opt_link = primary_entry.PointsTo()) {
            auto look_through_result = overflow_.LookThrough(entry, *opt_link);
            std::visit(
                overloaded{
                    [](const EntryAlreadyInOverflow &) {
                        fmt::print(
                            "Entry already in database overflow area.\n");
                    },
                    [&](const EmptyOverflow &) {
                        primary_entry = overflow_.Append(entry);
                        primary_.Insert(primary_entry, primary_link);
                        fmt::print("Inserted first to overflow: {}\nUpdated "
                                   "primary: {}\n",
                                   static_cast<std::string>(entry),
                                   static_cast<std::string>(primary_entry));
                    },
                    [&](const FirstWasLarger &order) {
                        entry = order.point;
                        primary_entry = overflow_.Append(entry);
                        primary_.Insert(primary_entry, primary_link);
                        fmt::print(
                            "Inserted to overflow: {}\nUpdated primary: {}\n",
                            static_cast<std::string>(entry),
                            static_cast<std::string>(primary_entry));
                    },
                    [&](const AppendWithoutPointing &order) {
                        auto updated_overflow_entry =
                            wr::get_ref<const Entry<Record>>(
                                overflow_.View(order.update));
                        updated_overflow_entry = overflow_.Append(entry);
                        overflow_.Insert(updated_overflow_entry, order.update);
                        fmt::print(
                            "Inserted to overflow: {}\nUpdated overflow: {}\n",
                            static_cast<std::string>(entry),
                            static_cast<std::string>(updated_overflow_entry));
                    },
                    [&](const AppendWithPointing &order) {
                        entry = order.point;
                        auto updated_overflow_entry =
                            wr::get_ref<const Entry<Record>>(
                                overflow_.View(order.update));
                        updated_overflow_entry = overflow_.Append(entry);
                        overflow_.Insert(updated_overflow_entry, order.update);
                        fmt::print(
                            "Inserted to overflow: {}\nUpdated overflow: {}\n",
                            static_cast<std::string>(entry),
                            static_cast<std::string>(updated_overflow_entry));
                    }},
                look_through_result);
        } else {
            primary_entry = overflow_.Append(entry);
            primary_.Insert(primary_entry, primary_link);
            fmt::print("Inserted to overflow: {}\nUpdated primary: {}\n",
                       static_cast<std::string>(entry),
                       static_cast<std::string>(primary_entry));
        }
    }

  public:
    void Setup(const std::string &prefix) {
        auto [primary_entries, overflow_entries] =
            index_.Setup(prefix + ".index");
        primary_.Setup(prefix + ".primary", primary_entries);
        if (primary_.Size() < 1) {
            primary_.Insert(Entry<Record>::dummy(), {0, 0});
        }
        overflow_.Setup(prefix + ".overflow", overflow_entries);
    }
    void Save() {
        index_.Serialize(primary_.Size(), overflow_.Size());
        primary_.Save();
        overflow_.Save();
    }
    void Insert(key::Key key, const Record &record) {
        db::Entry<Record> new_entry = {
            .key = key, .record = record, .link = {-1, -1}, .deleted = false};
        auto primary_page_link = index_.LookUp(key);
        try {
            auto look_through_result =
                primary_.LookThrough(new_entry, primary_page_link);
            std::visit(
                overloaded{
                    [&](const primary::EntryAlreadyInPrimary &) {
                        fmt::print("Entry already in database primary area.\n");
                    },
                    [&](const primary::EntryToPrimary &) {
                        AppendToPrimary(new_entry, primary_page_link);
                    },
                    [&](const primary::EntryToOverflow &order) {
                        AddToOverflow(new_entry, order.from_primary_start_link);
                    }},
                look_through_result);
        } catch (const std::exception &e) {
            throw std::runtime_error(
                fmt::format("Inserting error. Message: {}\n", e.what()));
        }
    }
};
} // namespace db
