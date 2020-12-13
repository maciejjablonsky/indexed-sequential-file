#ifndef DATABASE_HPP
#define DATABASE_HPP
#include <database/Record.hpp>
#include <database/Link.hpp>
#include <database/Key.hpp>
#include <utility>
#include <optional>
#include <functional>
namespace db

{
class DataBase
{
   public:
    std::optional<std::reference_wrapper<const area::Record>> Read(const area::Key key);
    void Write(const area::Key key, const area::Record& record);
    void Delete(const area::Key key);
    void Reorganize();
    std::pair<area::Key, area::Record> FetchNext(const area::Key key);
};
}  // namespace db

#endif  // DATABASE_HPP