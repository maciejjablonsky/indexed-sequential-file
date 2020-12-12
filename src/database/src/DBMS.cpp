#include <database/DBMS.hpp>
#include <fmt/format.h>
db::DBMS::DBMS(const std::string_view filenames_prefix, commands::source& src)
    : interpreter_(src)
{
}

void db::DBMS::Run() { fmt::print("DBMS running."); }