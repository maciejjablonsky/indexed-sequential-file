#ifndef DATABASE_INDEX_HPP
#define DATABASE_INDEX_HPP

#include <database/Key.hpp>
#include "Link.hpp"
#include <map>
namespace db
{
class Index
{
   public:
    Index() = default;
    Index(const std::string_view path);
    area::Link LookUp(const area::Key key) const;
    void Add(const area::Key key, const area::Link link);
    void Serialize(const std::string_view path) const;

   private:
    std::map<area::Key, area::Link> lookup_table_;
};
}  // namespace db

#endif  // DATABASE_INDEX_HPP