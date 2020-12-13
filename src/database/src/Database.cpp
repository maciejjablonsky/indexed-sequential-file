#include "Database.hpp"

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

db::DataBase::DataBase(const std::string_view filenames_prefix) {}

optref<const area::Record> db::DataBase::Read(const area::Key key)
{
    enum class entry_from
    {
        primary,
        overflow,
        none
    } from = entry_from::primary;
    auto link = index_.LookUp(key);
    auto last_entry_link = link;
    do
    {
        auto opt_entry = [&]() -> optref<const area::Entry> {
            switch (from)
            {
                case entry_from::primary:
                    return primary_.FetchNextEntry(link);
                case entry_from::overflow:
                    return overflow_.FetchEntry(link);
                default:
                    return std::nullopt;
            }
        }();
        if (opt_entry.has_value())
        {
            return std::nullopt;
        }
        const auto& entry = opt_entry->get();
        if (entry.key == key && !entry.deleted)
        {
            return entry.record;
        }
        if (entry.key > key)
        {
            from = [&]() {
                switch (from)
                {
                    case entry_from::primary:
                        return entry_from::overflow;
                    case entry_from::overflow:
                        return entry_from::none;
                    default:
                        return entry_from::none;
                }
            }();
        }
        last_entry_link = entry.link;
    } while (from != entry_from::none);
    return std::nullopt;
}

bool db::DataBase::Write(const area::Key key, const area::Record& record)
{
    auto result = Read(key);
    if (result.has_value())
    {
        return false;  // key already in database
    }
    auto page_link = index_.LookUp(key);
}
