#include <database/Key.hpp>
#include <fmt/format.h>

std::stringstream& area::operator>>(std::stringstream& ss, area::Key& key)
{
    ss >> key.value;
    return ss;
}

area::Key::operator std::string() const { return fmt::format("{}", value); }