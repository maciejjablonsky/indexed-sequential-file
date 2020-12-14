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


bool db::DataBase::Initialize(const std::string& name, const std::string_view config_path)
{
    std::filesystem::create_directory(name);
    auto path = fmt::format("{}/{}.", name, name);
    index_.Deserialize(path + "index");
    primary_.AttachFile(path + "primary", page_size_);
    overflow_.AttachFile(path + "overflow", page_size_);
    //std::ifstream ifs(config_path.data());
    //json j = json::parse(ifs);
    //try{
    //    autoreorganization_ = j["autoreorganization"];

    //}catch(const std::exception & e)
    //{
    //    fmt::print("Database configuration in invalid format. {}\n", e.what());
    //    return false;
    //}

    return true;
}

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
    auto link = index_.LookUp(key);
    auto opt_entry = primary_.FetchEntry(link);
    const auto &last_entry = opt_entry->get();
    while (opt_entry)
    {

    }
}
