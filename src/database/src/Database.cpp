#include "Database.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <fmt/format.h>
#include <filesystem>

using json = nlohmann::json;

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

bool db::DataBase::Initialize(const std::string& name,
                              const std::string_view config_path)
{
    std::filesystem::create_directory(name);
    auto path = fmt::format("{}/{}.", name, name);
    (void)index_.Load(path + "index", page_size_);
    primary_.AttachFile(path + "primary", page_size_);
    if (!primary_.ViewEntry({0, 0}))
    {
        primary_.AppendToPage(
            {.key = -1, .record = {0}, .link = {-1, -1}, .deleted = false},
            {0, 0});
    }
    overflow_.AttachFile(path + "overflow", page_size_);
    // std::ifstream ifs(config_path.data());
    // json j = json::parse(ifs);
    // try{
    //    autoreorganization_ = j["autoreorganization"];

    //}catch(const std::exception & e)
    //{
    //    fmt::print("Database configuration in invalid format. {}\n",
    //    e.what()); return false;
    //}

    return true;
}

optref<const area::Record> db::DataBase::Read(area::Key key)
{
    auto&& [opt_link, from] = FindEntry(key);
    switch (from)
    {
        case entry_from::primary:
            return primary_.ViewEntry(opt_link->get())->get().record;
        case entry_from::overflow:
            return overflow_.ViewEntry(opt_link->get())->get().record;
        default:
            return std::nullopt;
    }
}

bool db::DataBase::Insert(area::Key key, const area::Record& record)
{
    auto from = entry_from::primary;
    auto prev_from = entry_from::primary;
    auto link = index_.LookUp(key);
    auto opt_entry = primary_.ViewEntry(link);
    auto opt_prev_entry = opt_entry;
    area::Entry new_entry = {.key = key, .record = record, .link = {-1, -1}};
    do
    {
        if (opt_entry)
        {
            const auto& entry = opt_entry->get();

            if (entry.key == key && !entry.deleted)
            {
                return false;
            }
            else if (entry.key > key)
            {
                const auto& prev_entry = opt_prev_entry->get();
                if (from == entry_from::primary && !prev_entry.link.IsActive())
                {
                    new_entry.link = prev_entry.link;
                    primary_.FetchEntry(link)->get().link =
                        AppendToOverflow(new_entry);
                    return true;
                }
                else if (from == entry_from::overflow &&
                         prev_entry.link.IsActive())
                {
                    new_entry.link = prev_entry.link;
                    overflow_.FetchEntry(link)->get().link =
                        AppendToOverflow(new_entry);
                    return true;
                }
                else if (from == entry_from::primary &&
                         prev_entry.link.IsActive())
                {
                    from = entry_from::overflow;
                }
            }
            else if (from == entry_from::overflow)
            {
                link = entry.link;
            }
            opt_prev_entry = opt_entry;
            opt_entry = [&]() -> optref<const area::Entry> {
                if (from == entry_from::primary)
                {
                    auto&& [next_entry, next_link] =
                        primary_.ViewNextEntry(link);
                    link = next_entry ? next_link : link;
                    return std::move(next_entry);
                }
                else if (from == entry_from::overflow)
                {
                    return overflow_.ViewEntry(link);
                }
                else
                {
                    return std::nullopt;
                }
            }();
        }
        else
        {
            AssociateWithPage(new_entry, link);
            return true;
        }
    } while (from != entry_from::none);
    return false;
}

bool db::DataBase::Delete(area::Key key)
{
    auto&& [opt_link, from] = FindEntry(key);
    switch (from)
    {
        case entry_from::primary:
            primary_.FetchEntry(opt_link->get())->get().deleted = true;
            index_.Decrement(counter::primary_entries);
            return true;
        case entry_from::overflow:
            overflow_.FetchEntry(opt_link->get())->get().deleted = true;
            index_.Decrement(counter::overflow_entries);
            return true;
        default:
            return false;
    }
}

bool db::DataBase::Update(area::Key key, const area::Record& record)
{
    auto&& [opt_link, from] = FindEntry(key);
    switch (from)
    {
        case entry_from::primary:
            primary_.FetchEntry(opt_link->get())->get().record = record;
            return true;
        case entry_from::overflow:
            overflow_.FetchEntry(opt_link->get())->get().record = record;
            return true;
        default:
            return false;
    }
}

void db::DataBase::View()
{
    fmt::print("[{:^12}]\n", "PRIMARY");
    primary_.View();
    fmt::print("[{:^12}]\n", "OVERFLOW");
    overflow_.View();
}

std::pair<optref<area::Link>, db::entry_from> db::DataBase::FindEntry(
    area::Key key)
{
    auto from = db::entry_from::primary;
    auto link = index_.LookUp(key);
    do
    {
        auto opt_entry = [&]() {
            return from == db::entry_from::primary
                       ? primary_.ViewNextEntry(link).first
                       : overflow_.ViewEntry(link);
        }();
        if (!opt_entry.has_value())
        {
            return {std::nullopt, db::entry_from::none};
        }
        const auto& entry = opt_entry->get();
        if (entry.key == key && !entry.deleted)
        {
            return {link, from};
        }
        else if (entry.key > key)
        {
            from = [&]() {
                switch (from)
                {
                    case entry_from::primary:
                        return entry_from::overflow;
                    default:
                        return entry_from::none;
                }
            }();
        }
    } while (from != db::entry_from::none);

    return {std::nullopt, db::entry_from::none};
}

area::Link db::DataBase::AssociateWithPage(const area::Entry& entry,
                                           area::Link link)
{
    if (primary_.HowMuchPlaceLeft(link) > 0)
    {
        primary_.AppendToPage(entry, link);
        index_.Increment(counter::primary_entries);
    }
    else if (index_.IsLastPage(link))
    {
        auto link = primary_.InsertOnNewPage(entry);
        index_.AddPrimaryPage(entry.key, link);
        index_.Increment(counter::primary_entries);
        index_.Increment(counter::primary_pages);
    }
    else
    {
        auto&& [opt_primary_entry, primary_entry_link] =
            primary_.ViewLastEntryOnPage(link);
        if (!opt_primary_entry)
        {
            return {-1, -1};
        }
        if (opt_primary_entry->get().link.IsActive())
        {
        }
        else
        {
            auto opt_link = overflow_.Append(entry);
            if (opt_link)
            {
                primary_.FetchEntry(primary_entry_link)->get().link = *opt_link;
            }
            else
            {
                auto link = overflow_.InsertOnNewPage(entry);
                primary_.FetchEntry(primary_entry_link)->get().link = link;
                index_.Increment(counter::overflow_pages);
            }
            index_.Increment(counter::overflow_entries);
        }
    }
}

area::Link db::DataBase::AppendToOverflow(const area::Entry& entry)
{
    auto&& [opt_last_entry, last_entry_link] = overflow_.ViewLastEntry();
    auto&& [non_existing_entry, next_entry_link] =
        overflow_.ViewNextEntry(last_entry_link);
    if (non_existing_entry)
    {
        throw std::runtime_error("Entry at place where it should be empty.");
    }
    if (last_entry_link.page_no == next_entry_link.page_no)
    {
        (void)overflow_.Append(entry);
    }
    else
    {
        (void)overflow_.InsertOnNewPage(entry);
        index_.Increment(counter::overflow_pages);
    }
    index_.Increment(counter::overflow_entries);
    return next_entry_link;
}
