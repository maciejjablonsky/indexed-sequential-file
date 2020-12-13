#ifndef DATABASE_INDEX_HPP
#define DATABASE_INDEX_HPP

#include <database/Key.hpp>
#include "Link.hpp"

namespace db
{
class Index
{
   public:
    Index(int page_size);
    Index(const std::string_view path);
    area::Link LookUp(const area::Key key) const;
    void Serialize(const std::string_view path) const;

};
}  // namespace db

#endif  // DATABASE_INDEX_HPP