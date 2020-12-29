#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <overloaded/overloaded.hpp>
#include <string>
#include <type_traits>
#include <variant>

namespace link {
struct PageLink {
    int32_t index;
    operator decltype(index)() const { return index; }
    template <typename T>
    requires std::is_integral_v<T> inline PageLink operator+(T val) {
        return {index + val};
    }
};
struct PrimaryPageLink : public PageLink {};

struct OverflowPageLink : public PageLink {};

struct LastPrimaryPageLink : public PageLink {};

struct EntryLink {
    int32_t page;
    int32_t entry;
    operator bool() const { return page >= 0 && entry >= 0; }
    operator std::string() const {
        return fmt::format("{{page: {:>4}, entry: {:>4}}}", page, entry);
    }
};

struct PrimaryEntryLink : public EntryLink {
    operator EntryLink() const { return {page, entry}; }
};

struct OverflowEntryLink : public EntryLink {
    operator EntryLink() const { return {page, entry}; }
};

} // namespace link
