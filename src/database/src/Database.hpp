#pragma once

#include <database/Record.hpp>
#include <database/Key.hpp>
#include <utility>
#include <optional>
#include "Index.hpp"
#include <concepts>
#include <type_traits>
#include <concepts/comparable.hpp>

template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;
namespace db
{

template <typename Key, typename Record>
concept database_concept = requires
{
    comparable<Key>, std::is_trivial_v<Record>;
};

template <typename Key, typename Record>
requires database_concept<Key, Record> class DataBase
{
   public:
    bool Setup(const std::string& prefix);

   private:
    float autoreorganization_ = 0.2;
    float page_utilization_ = 0.5;
    int page_size_ = 4096;
    index::Index<Key, Record> index_;
};
template <typename Key, typename Record>
requires database_concept<Key, Record> inline bool DataBase<Key, Record>::Setup(const std::string& prefix)
{
    return index_.Setup(prefix + ".index");
}
}  // namespace db
