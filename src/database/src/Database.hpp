#pragma once

#include <database/Record.hpp>
#include <database/Key.hpp>
#include <utility>
#include <optional>
#include "Index.hpp"
#include <concepts>
#include <type_traits>
#include <concepts/comparable.hpp>
#include "Link.hpp"

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
    void Setup(const std::string& prefix);
    void Save();

   private:
    float autoreorganization_ = 0.2;
    float page_utilization_ = 0.5;
    index::Index<key::IndexKey, link::PageLink> index_;
};
template <typename Key, typename Record>
requires database_concept<Key, Record> inline void DataBase<Key, Record>::Setup(const std::string& prefix)
{
    auto saved_entries = index_.Setup(prefix + ".index");
}
template <typename Key, typename Record>
requires database_concept<Key, Record> inline void DataBase<Key, Record>::Save()
{
    index_.Serialize(0xdeadbeef, 0xcafecafe);
}
}  // namespace db
