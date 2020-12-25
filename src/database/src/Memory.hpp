#pragma once

#include <array>
#include <cstdint>

namespace page
{

constexpr auto DEFAULT_PAGE_MEMORY = 4096U;

template <size_t size>
using continuous_memory = std::array<std::byte, size>;

template <size_t page_size = DEFAULT_PAGE_MEMORY>
class PageMemory : public continuous_memory<page_size>
{
   public:
    static constexpr size_t size = page_size;
    PageMemory(size_t index);
    PageMemory(size_t index, continuous_memory<page_size>&& memory);
    inline auto Index() const { return index_; }

   private:
    size_t index_;
};

template <size_t page_size> 
inline PageMemory<page_size>::PageMemory(size_t index) : continuous_memory<page_size>(), index_(index)
{
}
template <size_t page_size>
inline PageMemory<page_size>::PageMemory(size_t index, continuous_memory<page_size>&& memory)
    : continuous_memory<page_size>(std::move(memory)), index_(index)
{
}
}  // namespace page
