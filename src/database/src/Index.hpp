#ifndef DATABASE_INDEX_HPP
#define DATABASE_INDEX_HPP

#include <database/Key.hpp>
#include <database/Link.hpp>

namespace db
{
class Index
{
   public:
    area::Link LookUp(const area::Key key) const;
};
}  // namespace db

#endif  // DATABASE_INDEX_HPP