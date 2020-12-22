#pragma once
#include <type_traits>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include "DiskAccess.hpp"

namespace page
{
constexpr auto PAGE_SIZE = 4096U;

class PageMemory : public std::vector<std::byte>
{
   public:
    PageMemory(size_t index, size_t size);
    PageMemory(size_t index, std::vector<std::byte>&& memory);
    inline auto Index() const { return index_; }
   private:
    size_t index_;
};

class PageDispositor
{
   public:
    PageDispositor(const std::string& filename);
    PageMemory Request(size_t page_index);
    PageMemory MakePage();
    void Write(const PageMemory& page);
    [[nodiscard]] inline auto CountReads() const
    {
        return counter_.CountReads();
    }
    [[nodiscard]] inline auto CountWrites() const
    {
        return counter_.CountWrites();
    }

   private:
    std::string file_path_;
    size_t pages_in_file_;
    std::unique_ptr<std::fstream> file_;
    DiskAccess counter_;
};

static_assert(std::is_move_assignable<PageDispositor>());
}  // namespace page