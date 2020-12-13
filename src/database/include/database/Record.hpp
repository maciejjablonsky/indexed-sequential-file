#ifndef DATABASE_AREA_RECORD_HPP
#define DATABASE_AREA_RECORD_HPP

#include <cstdint>
#include <compare>
#include <sstream>

namespace area {
    struct Record{
        uint64_t time = 0;
        friend std::stringstream& operator>>(std::stringstream& ss,
                                             area::Record& record);
        operator std::string() const;
    };
}

#endif // DATABASE_AREA_RECORD_HPP