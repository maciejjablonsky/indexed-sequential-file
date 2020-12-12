#ifndef DATABASE_HPP
#define DATABASE_HPP
#include <database/Record.hpp>
#include <database/Link.hpp>
#include <database/Key.hpp>
#include <utility>

namespace db

{
class DataBase
{
   public:
    const area::Record& Read(const area::Link link);
    void Write(const area::Key key, const area::Record& record);
    void Delete(const area::Key key);
    void Reorganize();
    std::pair<area::Key, area::Record> FetchNext(const area::Key key);
};
}  // namespace db

#endif  // DATABASE_HPP