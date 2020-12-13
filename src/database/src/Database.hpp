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
    optref<const area::Record> Read(const area::Key key);
    bool Write(const area::Key key, const area::Record& record);
    void Delete(const area::Key key);
    void Reorganize();

   private:
    db::Index index_;
    area::Area primary_;
    area::Area overflow_;
};
}  // namespace db

#endif  // DATABASE_HPP