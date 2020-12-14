#ifndef DATABASE_INDEX_HPP
#define DATABASE_INDEX_HPP

#include <database/Key.hpp>
#include "Link.hpp"
#include <vector>
#include <utility>
namespace db
{
struct key_link
{
    area::Key key;
    area::Link link;
};
class Index
{
   public:
    [[nodiscard]] area::Link LookUp(const area::Key key) const;
    void Add(const area::Key key, const area::Link link);
    void Serialize(const std::string& path) const;
    void Deserialize(const std::string& path);

   private:
    std::vector<key_link> lookup_table_;
};
}  // namespace db

#endif  // DATABASE_INDEX_HPP