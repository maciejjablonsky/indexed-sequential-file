#include "Database.hpp"

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

optref<const area::Record> db::DataBase::Read(const area::Key key)
{
    auto link = index_.LookUp(key);
    auto result =
        std::make_unique<area::record_or_link>(primary_.FetchRecord(link));
    while (std::holds_alternative<const area::Link>(*result))
    {
       result = std::make_unique<area::record_or_link>(std::visit(
            overloaded{[](optref<const area::Record>& opt) -> area::record_or_link { return opt; },
                       [&](const area::Link& link) {
                           return overflow_.FetchRecord(link);
                       }},
            *result));
    }
    return std::get<optref<const area::Record>>(*result);
}

bool db::DataBase::Write(const area::Key key, const area::Record& record) {
    auto result = Read(key);
    if (result.has_value())
    {
        return false; // key already in database
    }
    auto page_link = index_.LookUp(key);
}

