#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace page {

constexpr auto DEFAULT_PAGE_MEMORY = 4096U;

using continuous_memory = std::vector<std::byte>;

template <size_t page_size = DEFAULT_PAGE_MEMORY>
class PageMemory : public continuous_memory {
  public:
    static constexpr size_t size = page_size;
    PageMemory(size_t index) : continuous_memory(page_size), index_(index) {}
    PageMemory(size_t index, continuous_memory &&memory)
        : continuous_memory(std::move(memory)), index_(index) {}

    inline auto Index() const { return index_; }

    template <typename T> const T &ViewAs(size_t offset) const {
        return static_cast<const T &>(
            reinterpret_cast<const T *>(data() + offset));
    }

  private:
    size_t index_;
};
} // namespace page
