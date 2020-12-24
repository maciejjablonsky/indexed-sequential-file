#pragma once

#include <type_traits>
#include <array>
#include <fstream>
#include <string>
#include <memory>
#include "DiskAccess.hpp"
#include <concepts/memory_access.hpp>
#include <stdexcept>

namespace page
{

template <typename Memory>
requires memory_access<Memory> class PageDispositor
{
   public:
    bool Setup(const std::string& file_path);
    Memory Request(size_t page_index);
    void Write(const Memory& page);
    inline size_t PagesInFile() const { return pages_in_file_; }
    [[nodiscard]] inline auto CountReads() const { return counter_.CountReads(); }
    [[nodiscard]] inline auto CountWrites() const { return counter_.CountWrites(); }
    void ClearFile();

   private:
    Memory Make();
    std::string file_path_;
    size_t pages_in_file_;
    std::unique_ptr<std::fstream> file_;
    DiskAccess counter_;
};

template <typename Memory>
requires memory_access<Memory> inline bool PageDispositor<Memory>::Setup(const std::string& file_path)
{
    file_path_ = file_path;
    file_ = std::make_unique<std::fstream>(file_path, std::ios::binary);
    pages_in_file_ = std::filesystem::exists(file_path) ? std::filesystem::file_size(file_path) / Memory::size : 0;
    return true;
}

template <typename Memory>
requires memory_access<Memory> Memory page::PageDispositor<Memory>::Request(size_t page_index)
{
    if (!file_ || !file_->is_open())
    {
        throw std::runtime_error("No file set to page dispositor.");
    }
    if (page_index > pages_in_file_)
    {
        throw std::out_of_range("Requested page behind page index.");
    }
    else if (page_index == pages_in_file_)
    {
        return Make();
    }
    else
    {
        file_->seekg(page_index*);
        std::vector<std::byte> tmp(Memory::size);
        file_->read(reinterpret_cast<char*>(tmp.data()), Memory::size);
        counter_.Read();
        return {page_index, std::move(tmp)};
    }
}

template <typename Memory>
requires memory_access<Memory> inline void PageDispositor<Memory>::ClearFile()
{
    file_ = std::make_unique<std::fstream>(file_path_, std::ios::binary | std::ios::trunc);
    pages_in_file_ = 0;
}

template <typename Memory>
requires memory_access<Memory> inline Memory PageDispositor<Memory>::Make()
{
    auto tmp = Memory(pages_in_file_);
    ++pages_in_file_;
    return tmp;
}

template <typename Memory>
requires memory_access<Memory> inline void PageDispositor<Memory>::Write(const Memory& page)
{
    if (!file_ || !file_->is_open())
    {
        throw std::runtime_error("No file set to page dispositor.");
    }
    file_->seekg(page.Index() * Memory::size);
    file_->write(reinterpret_cast<const char*>(page.data()), Memory::size);
    counter_.Write();
}

}  // namespace page