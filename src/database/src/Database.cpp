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
    index_.Deserialize(path + "index");
    primary_.AttachFile(path + "primary", page_size_);
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
        case entry::primary:
            return primary_.ViewEntry(opt_link->get())->get().record;
        case entry::overflow:
            return overflow_.ViewEntry(opt_link->get())->get().record;
        default:
            return std::nullopt;
    }
}

bool db::DataBase::Insert(area::Key key, const area::Record& record)
{

}

bool db::DataBase::Delete(area::Key key)
{
    auto&& [opt_link, from] = FindEntry(key);
    switch (from)
    {
        case entry::primary:
            primary_.FetchEntry(opt_link->get())->get().deleted = true;
            return true;
        case entry::overflow:
            overflow_.FetchEntry(opt_link->get())->get().deleted = true;
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
        case entry::primary:
            primary_.FetchEntry(opt_link->get())->get().record = record;
            return true;
        case entry::overflow:
            overflow_.FetchEntry(opt_link->get())->get().record = record;
            return true;
        default:
            return false;
    }
}

std::pair<optref<area::Link>, db::entry> db::DataBase::FindEntry(area::Key key)
{
    auto from = db::entry::primary;
    auto link = index_.LookUp(key);
    do
    {
        auto opt_entry = [&]() {
            return from == db::entry::primary ? primary_.ViewNextEntry(link)
                                              : overflow_.ViewEntry(link);
        }();
        if (!opt_entry.has_value())
        {
            return {std::nullopt, db::entry::none};
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
                    case entry::primary:
                        return entry::overflow;
                    default:
                        return entry::none;
                }
            }();
        }
    } while (from != db::entry::none);

    return {std::nullopt, db::entry::none};
}