#pragma once

#include <cstdint>
#include <variant>

namespace link
{
struct DisabledLink
{
};

struct ActiveLink
{
    const uint32_t page;
    const uint32_t entry;
};

struct PODLink
{
    uint32_t page;
    uint32_t entry;
};

using EntryLink = std::variant<DisabledLink, ActiveLink>;

using PageLink = size_t;


}  // namespace links
