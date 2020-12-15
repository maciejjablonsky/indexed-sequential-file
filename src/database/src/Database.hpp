#ifndef DATABASE_HPP
#define DATABASE_HPP
#include <database/Record.hpp>
#include "Link.hpp"
#include <database/Key.hpp>
#include <utility>
#include <optional>
#include <functional>
#include "Index.hpp"
#include "Area.hpp"
template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;
namespace db

{
enum class entry
{
    primary,
    overflow,
    none
};
class DataBase
{

   public:
    [[nodiscard]] bool Initialize(
        const std::string& name,
        const std::string_view config_path = "database_config.json");
    [[nodiscard]] optref<const area::Record> Read(area::Key key);
    [[nodiscard]] bool Insert(area::Key key, const area::Record& record);
    [[nodiscard]] bool Delete(area::Key key);
    void Reorganize();
    [[nodiscard]] bool Update(area::Key key, const area::Record& record);

   private:
    [[nodiscard]] std::pair<optref<area::Link>, db::entry> FindEntry(area::Key key);
    void AppendEntry(const area::Entry& entry, db::entry destination);
    float autoreorganization_ = 0.2;
    float page_utilization_ = 0.5;
    int page_size_ = 4096;
    db::Index index_;
    area::Area primary_;
    area::Area overflow_;
};
}  // namespace db

#endif  // DATABASE_HPP