#pragma once

#include <cstdint>
#include <compare>
#include <sstream>
#include <type_traits>
#include <fmt/format.h>
namespace record
{
struct Record
{
    uint64_t time;
    friend std::stringstream& operator>>(std::stringstream& ss, record::Record& record);
    inline operator std::string() const { return fmt::format("{}", time); }
};

inline std::stringstream& operator>>(std::stringstream& ss, record::Record& record)
{
    ss >> record.time;
    return ss;
}
static_assert(std::is_trivial<Record>());
}  // namespace record
