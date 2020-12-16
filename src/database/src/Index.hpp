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
    int primary_entries;
    int overflow_entries;
    int page_size;
};
struct IndexPageHeader
{
    int entries_number;
};
enum class counter
{
    primary_pages,
    overflow_pages,
    primary_entries,
    overflow_entries,
    none
};
class Index
{
   public:
    [[nodiscard]] area::Link LookUp(const area::Key key) const;
    void AddPrimaryPage(const area::Key key, const area::Link link);
    void Serialize(const std::string& path) const;
    [[nodiscard]] bool Load(const std::string& path, int page_size);
    [[nodiscard]] bool Deserialize(const std::string& path, int page_size);
    void Increment(counter which);
    void Decrement(counter which);
    [[nodiscard]] bool IsLastPage(area::Link link);

   private:
    void Reset(int page_size);
    IndexHeader header_;
    std::vector<key_link> lookup_table_;
};
}  // namespace db

#endif  // DATABASE_INDEX_HPP