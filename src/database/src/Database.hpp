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
class DataBase
{

   public:
    DataBase(const std::string_view filenames_prefix);
    bool Initialize(const std::string& config_path = "database_config.json");
    optref<const area::Record> Read(const area::Key key);
    bool Write(const area::Key key, const area::Record& record);
    void Delete(const area::Key key);
    void Reorganize();

   private:
    float autoreorganization_ = 0.2;
    float page_utilization_ = 0.5;
    int page_size_ = 4096;
    db::Index index_;
    area::Area primary_;
    area::Area overflow_;
};
}  // namespace db

#endif  // DATABASE_HPP