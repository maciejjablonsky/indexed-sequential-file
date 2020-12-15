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
struct IndexHeader
{
    int primary_pages;
    int overflow_pages;
    int index_pages;
    int primary_entries;
    int overflow_entries;
    int page_size;
};
struct IndexPageHeader
{
    int entries_number;
};
class Index
{
   public:
    [[nodiscard]] area::Link LookUp(const area::Key key) const;
    void Add(const area::Key key, const area::Link link);
    void Serialize(const std::string& path) const;
    void Deserialize(const std::string& path, int page_size);

   private:
    void Load(const std::string& path, int page_size);
    void Reset(int page_size);
    IndexHeader header_;
    std::vector<key_link> lookup_table_;
};
}  // namespace db

#endif  // DATABASE_INDEX_HPP