#include "Area.hpp"
#include <filesystem>

void area::Area::AttachFile(std::string_view path, int page_size)
{
    page_size_ = page_size;
    file_ = std::make_unique<std::fstream>(path.data(), std::ios::binary);
    auto file_exists = std::filesystem::exists(path);
    if ((file_exists && std::filesystem::file_size(path) == 0) || !file_exists)
    {
        current_page_ = EntryPage(page_size_);
        current_page_idx_ = 0;
        pages_number_ = 0;
        current_page_->Write({.key = {.value = -1},
                              .record = {.time = static_cast<uint64_t>(-1)},
                              .link = {-1, -1}});
    }
    else
    {
        auto file_size = std::filesystem::file_size(path);
        pages_number_ = file_size / page_size_;
        current_page_idx_ = -1;
        current_page_ = std::nullopt;
    }
}

area::Link area::Area::CreatePageWithEntry(area::Entry&& entry)
{
    if (current_page_ && current_page_->IsDirty())
    {
        WriteDiskPage();
    }
    current_page_ = area::EntryPage(page_size_);
    current_page_idx_ = pages_number_;
    ++pages_number_;
    current_page_->Write(std::move(entry));
    return {current_page_idx_, 0};
}

optref<const area::Entry> area::Area::FetchEntry(const area::Link link)
{
    if (link.page_no != current_page_idx_)
    {
        if (current_page_->IsDirty())
        {
            WriteDiskPage();
        }
        auto opt_page = ReadDiskPage(link.page_no);
        if (!opt_page)
        {
            return std::nullopt;
        }
        current_page_ = EntryPage(std::move(*opt_page));
        current_page_idx_ = link.page_no;
    }
    if (current_page_)
    {
        return current_page_->Read(link.entry_index);
    }
    return std::nullopt;
}

optref<const area::Entry> area::Area::FetchNextEntry(const area::Link link)
{
    const_cast<area::Link&>(link).entry_index += 1;
    return FetchEntry(link);
}

std::optional<std::vector<std::byte>> area::Area::ReadDiskPage(int idx)
{
    std::vector<std::byte> tmp(page_size_);
    file_->read(reinterpret_cast<char*>(tmp.data()), page_size_);
    if (file_->eof())
    {
        return std::nullopt;
    }
    return std::move(tmp);
}

void area::Area::WriteDiskPage()
{
    if (!file_)
    {
        throw std::runtime_error("No file set to write.");
    }
    if (!current_page_)
    {
        throw std::runtime_error("No page set to write");
    }
    file_->seekg(current_page_idx_ * page_size_);
    file_->write(reinterpret_cast<char*>(current_page_->Data()), page_size_);
    current_page_->ClearDirty();
}
