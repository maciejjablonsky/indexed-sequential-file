#include "Record.hpp"
#include <fmt/format.h>

std::stringstream& area::operator>>(std::stringstream& ss, area::Record& record)
{
    ss >> record.time;
    return ss;
}

area::Record::operator std::string() const { return fmt::format("{}", time); }
