#include "PageDispositor.hpp"
#include <filesystem>

page::PageDispositor::PageDispositor(const std::string& file_path)
    : file_path_(file_path),
      file_(std::make_unique<std::fstream>(file_path, std::ios::binary)),
      pages_in_file_(std::filesystem::exists(file_path)
                         ? std::filesystem::file_size(file_path) / PAGE_SIZE
                         : 0)
{
}

page::PageMemory page::PageDispositor::Request(size_t page_index)
{
    if (!file_ || !file_->is_open())
    {
        file_ = std::make_unique<std::fstream>(file_path_, std::ios::binary);
        pages_in_file_ =
            std::filesystem::exists(file_path_)
                ? std::filesystem::file_size(file_path_) / PAGE_SIZE
                : 0;
    }
    if (page_index >= pages_in_file_)
    {
        throw std::out_of_range("Requested page behind page index.");
    }
    file_->seekg(page_index * PAGE_SIZE);
    std::vector<std::byte> tmp(PAGE_SIZE);
    file_->read(reinterpret_cast<char*>(tmp.data()), PAGE_SIZE);
    counter_.Read();
    return {page_index, std::move(tmp)};
}

page::PageMemory page::PageDispositor::MakePage()
{
    auto new_page = PageMemory(pages_in_file_, PAGE_SIZE);
    ++pages_in_file_;
    return new_page;
}

void page::PageDispositor::Write(const PageMemory& page) {
    if (!file_ || !file_->is_open())
    {
        file_ = std::make_unique<std::fstream>(file_path_, std::ios::binary);
        pages_in_file_ =
            std::filesystem::exists(file_path_)
                ? std::filesystem::file_size(file_path_) / PAGE_SIZE
                : 0;
    }
    file_->seekg(page.Index() * PAGE_SIZE);
    file_->write(reinterpret_cast<const char*>(page.data()), PAGE_SIZE);
    counter_.Write();
}

page::PageMemory::PageMemory(size_t index, size_t size)
    : std::vector<std::byte>(size), index_(index)
{
}

page::PageMemory::PageMemory(size_t index, std::vector<std::byte>&& memory)
    : std::vector<std::byte>(std::move(memory)), index_(index)
{
}
