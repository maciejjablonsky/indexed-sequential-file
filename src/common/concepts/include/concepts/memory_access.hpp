#pragma once
#include <concepts>

template <typename Memory> concept memory_access = requires(Memory mem) {
    Memory::size, mem.data(), mem.begin(), mem.end();
};