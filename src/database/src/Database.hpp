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
#include <nlohmann/json.hpp>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <wrappers/optref.hpp>

using json = nlohmann::json;

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
    std::string prefix_;
    float autoreorganization_ = 0.2;
    float page_utilization_ = 0.5;
    index::Index<key::Key> index_;
    primary::Primary<Entry<Record>> primary_;
    overflow::Overflow<Entry<Record>> overflow_;

    using search_result = std::tuple<
        wr::optional_ref<const Entry<Record>>,
        std::variant<link::PrimaryEntryLink, link::OverflowEntryLink>>;

    class SequentialSearcher {
      private:
        DataBase<Record> &db_;
        link::PrimaryEntryLink last_primary_link_ = {0, 0};
        std::variant<link::PrimaryEntryLink, link::OverflowEntryLink>
            next_link_ = last_primary_link_;

      public:
        SequentialSearcher(DataBase<Record> &db) : db_(db) {}
        search_result NextEntry() {
            auto next_primary_link = [&](const auto &link) {
                link::PrimaryEntryLink primary_link = link;
                if (link.entry < db_.primary_.LoadedPageSize() - 1) {
                    primary_link.entry += 1;
                } else {
                    primary_link.page += 1;
                    primary_link.entry = 0;
                }
                return primary_link;
            };

            return std::visit(
                overloaded{
                    [&](link::PrimaryEntryLink &primary_link) {
                        last_primary_link_ = primary_link;
                        auto opt_entry = db_.primary_.View(primary_link);
                        search_result result = {opt_entry, primary_link};
                        if (opt_entry) {
                            const auto &entry =
                                wr::get_ref<const Entry<Record>>(opt_entry);
                            if (auto opt_link = entry.PointsTo()) {
                                next_link_ = link::OverflowEntryLink{
                                    opt_link->page, opt_link->entry};
                            } else {
                                next_link_ =
                                    next_primary_link(last_primary_link_);
                            }
                            if (entry.IsDeleted()) {
                                return NextEntry();
                            }
                        }
                        return result;
                    },
                    [&](link::OverflowEntryLink &overflow_link) {
                        auto opt_entry = db_.overflow_.View(overflow_link);
                        search_result result = {opt_entry, overflow_link};
                        if (opt_entry) {
                            const auto &entry =
                                wr::get_ref<const Entry<Record>>(opt_entry);
                            if (auto opt_link = entry.PointsTo()) {
                                overflow_link = *opt_link;
                            } else {
                                next_link_ =
                                    next_primary_link(last_primary_link_);
                            }
                            if (entry.IsDeleted()) {
                                return NextEntry();
                            }
                        }
                        return result;
                    }},
                next_link_);
        }
    };

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
        prefix_ = prefix;
        std::ifstream ifs("database.config");
        if (!ifs.is_open()) {
            throw std::runtime_error(
                "Config file database.config not found in working directory.");
        }
        json config = json::parse(ifs);
        autoreorganization_ = config["autoreorganization"].get<float>();
        page_utilization_ = config["page_utilization"].get<float>();
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
            if (overflow_.Size() > primary_.Size() * autoreorganization_) {
                Reorganize();
            }

        } catch (const std::exception &e) {
            throw std::runtime_error(
                fmt::format("Inserting error. Message: {}\n", e.what()));
        }
    }
    void Show() {
        index_.Show();
        fmt::print("[{:^91}]\n",
                   fmt::format("PRIMARY   size: {}", primary_.Size()));
        primary_.Show();
        fmt::print("[{:^91}]\n",
                   fmt::format("OVERFLOW   size: {}", overflow_.Size()));
        overflow_.Show();
    }

    void ShowSorted() {
        index_.Show();
        SequentialSearcher searcher(*this);
        while (true) {
            auto [opt_entry, link] = searcher.NextEntry();
            if (!opt_entry) {
                break;
            }
            const auto &entry = wr::get_ref<const Entry<Record>>(opt_entry);
            std::visit(
                overloaded{
                    [&](const link::PrimaryEntryLink &primary_link) {
                        fmt::print("PRIMARY  {} | {}\n",
                                   static_cast<std::string>(primary_link),
                                   static_cast<std::string>(entry));
                    },
                    [&](const link::OverflowEntryLink &overflow_link) {
                        fmt::print("OVERFLOW {} | {}\n",
                                   static_cast<std::string>(overflow_link),
                                   static_cast<std::string>(entry));
                    }},
                link);
        }
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

    void Reorganize() {
        index_.Clear();
        auto new_primary = std::make_unique<primary::Primary<Entry<Record>>>();
        auto reorganized_filename = prefix_ + "_reorganized.primary";
        new_primary->Setup(reorganized_filename);

        std::variant<link::PrimaryEntryLink, link::OverflowEntryLink>
            var_entry_link = link::PrimaryEntryLink{0, 0};
        link::PrimaryPageLink appending_page = {0};
        index_.Add(key::Key::dummy(), appending_page);
        SequentialSearcher searcher(*this);
        while (true) {
            auto [opt_entry, var_link] = searcher.NextEntry();
            if (!opt_entry) {
                break;
            }
            auto entry = wr::get_ref<const Entry<Record>>(opt_entry);
            entry.link = link::EntryLink::Default();
            if (new_primary->LoadedPageSize() >=
                page_utilization_ * new_primary->SinglePageCapacity()) {
                ++appending_page;
                index_.Add(entry.key, appending_page);
            }
            new_primary->Append(entry, appending_page);
        }
        new_primary->Save();
        auto new_disk_accesses = new_primary->GetDiskAccesses();
        size_t stored_entries = new_primary->Size();
        new_primary.reset();
        overflow_.Clear();
        auto primary_filename = prefix_ + ".primary";
        primary_.RenameAndSwapFile(reorganized_filename, primary_filename,
                                   new_disk_accesses, stored_entries);
    }

    void DumpDiskAccessMetric(std::stringstream &ss) {
        // primary read, primary write, primary sum, overflow read, overflow
        // write, overflow sum, reads sum, writes sum, all sum
        const auto &primary_accesses = primary_.GetDiskAccesses();
        const auto &overflow_accesses = overflow_.GetDiskAccesses();
        ss << fmt::format("{},{},{},{},{},{},{},{},{}\n",
                          primary_accesses.reads, primary_accesses.writes,
                          primary_accesses.reads + primary_accesses.writes,
                          overflow_accesses.reads, overflow_accesses.writes,
                          overflow_accesses.reads + overflow_accesses.writes,
                          primary_accesses.reads + overflow_accesses.reads,
                          primary_accesses.writes + overflow_accesses.writes,
                          primary_accesses.reads + primary_accesses.writes +
                              overflow_accesses.reads +
                              overflow_accesses.writes);
    }
};
} // namespace db
