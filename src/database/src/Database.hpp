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
#include <tuple>
#include <type_traits>
#include <utility>
#include <wrappers/optref.hpp>

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
    auto operator<=>(const key::Key &key) const { return this->key <=> key; }
    auto operator==(const key::Key &key) const { return this->key == key; }
    static Entry dummy() {
        Entry dummy = {.key = {-1}, .link = {-1, -1}};
        memset(&dummy.record, 0x00, sizeof(Record));
        return dummy;
    }
    operator std::string() const {
        return fmt::format(
            "{{key: {}, content: {:>8}, pointing_to: {}, deleted: {:>6}}}",
            static_cast<std::string>(key), static_cast<std::string>(record),
            static_cast<std::string>(link), deleted);
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
                    },
                    [&](const FirstWasLarger &order) {
                        entry = order.point;
                        primary_entry = overflow_.Append(entry);
                        primary_.Insert(primary_entry, primary_link);
                    },
                    [&](const AppendWithoutPointing &order) {
                        auto updated_overflow_entry =
                            wr::get_ref<const Entry<Record>>(
                                overflow_.View(order.update));
                        updated_overflow_entry = overflow_.Append(entry);
                        overflow_.Insert(updated_overflow_entry, order.update);
                    },
                    [&](const AppendWithPointing &order) {
                        entry = order.point;
                        auto updated_overflow_entry =
                            wr::get_ref<const Entry<Record>>(
                                overflow_.View(order.update));
                        updated_overflow_entry = overflow_.Append(entry);
                        overflow_.Insert(updated_overflow_entry, order.update);
                    }},
                look_through_result);
        } else {
            primary_entry = overflow_.Append(entry);
            primary_.Insert(primary_entry, primary_link);
        }
    }

    using search_result = std::tuple<
        wr::optional_ref<const Entry<Record>>,
        std::variant<link::PrimaryEntryLink, link::OverflowEntryLink>>;
    search_result FindEntry(key::Key key) {
        auto primary_search_result =
            primary_.LookThrough(key, index_.LookUp(key));
        return std::visit(
            overloaded{
                [&](const primary::EntryAlreadyInPrimary &message) {
                    link::PrimaryEntryLink primary_link = {message.link.page,
                                                           message.link.entry};
                    return search_result{primary_.View(primary_link),
                                         primary_link};
                },
                [&](const primary::EntryNotFound &) {
                    return search_result{std::nullopt,
                                         link::PrimaryEntryLink{-1, -1}};
                },
                [&](const primary::EntryMightBeInOverflow &message) {
                    auto overflow_search_result =
                        overflow_.LookThrough(key, message.start_link);
                    return std::visit(
                        overloaded{[&](const overflow::EntryAlreadyInOverflow
                                           &message) {
                                       link::OverflowEntryLink overflow_link = {
                                           message.link.page,
                                           message.link.entry};
                                       return search_result{
                                           overflow_.View(overflow_link),
                                           overflow_link};
                                   },
                                   [&](const overflow::EntryNotFound &) {
                                       return search_result{
                                           std::nullopt,
                                           link::OverflowEntryLink{-1, -1}};
                                   }},
                        overflow_search_result);
                }},
            primary_search_result);
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
    void Insert(const key::Key &key, const Record &record) {
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
    void Show() {
        index_.Show();
        fmt::print("[{:^131}]\n", "PRIMARY");
        primary_.Show();
        fmt::print("[{:^131}]\n", "OVERFLOW");
        overflow_.Show();
    }

    void Read(const key::Key &key) {
        auto &&[opt_entry, var_link] = FindEntry(key);
        if (opt_entry) {
            const auto &entry = wr::get_ref<const Entry<Record>>(opt_entry);
            std::visit(
                overloaded{[&](const link::PrimaryEntryLink &link) {
                               fmt::print("Entry found in primary.\nlocation: "
                                          "{}, entry: {}\n\n",
                                          static_cast<std::string>(link),
                                          static_cast<std::string>(entry));
                           },
                           [&](const link::OverflowEntryLink &link) {
                               fmt::print("Entry found in overflow.\nlocation: "
                                          "{}, entry: {}\n\n",
                                          static_cast<std::string>(link),
                                          static_cast<std::string>(entry));
                           }},
                var_link);
        } else {
            fmt::print("Entry not found.\n\n");
        }
    }

    void Update(const key::Key &key, const Record &record) {
        auto &&[opt_entry, var_link] = FindEntry(key);
        if (opt_entry) {
            auto updated_entry = wr::get_ref<const Entry<Record>>(opt_entry);
            updated_entry.record = record;
            std::visit(overloaded{[&](const link::PrimaryEntryLink &link) {
                                      primary_.Insert(updated_entry, link);
                                  },
                                  [&](const link::OverflowEntryLink &link) {
                                      overflow_.Insert(updated_entry, link);
                                  }},
                       var_link);
        } else {
            fmt::print("Entry with key [{}] not found.\n",
                       static_cast<std::string>(key));
        }
    }

    void Delete(const key::Key &key) {
        auto &&[opt_entry, var_link] = FindEntry(key);
        if (opt_entry) {
            auto updated_entry = wr::get_ref<const Entry<Record>>(opt_entry);
            updated_entry.deleted = true;
            std::visit(overloaded{[&](const link::PrimaryEntryLink &link) {
                                      primary_.Insert(updated_entry, link);
                                  },
                                  [&](const link::OverflowEntryLink &link) {
                                      overflow_.Insert(updated_entry, link);
                                  }},
                       var_link);
        } else {
            fmt::print("Entry with key [{}] not found.\n",
                       static_cast<std::string>(key));
        }
    }
};
} // namespace db
