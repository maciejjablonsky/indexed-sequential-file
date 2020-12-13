#include "Area.hpp"

area::Area::Area(std::string_view path, int page_size)
    : file_(std::make_unique<std::fstream>(path.data(), std::ios::binary)),
      current_page_idx_(-1),
      page_size_(page_size),
      current_page_(std::nullopt)
{
}

void area::Area::AttachFile(std::string_view path)
{
    file_ = std::make_unique<std::fstream>(path.data(), std::ios::binary);
    current_page_ = -1;
    current_page_ = std::nullopt;
}

optref<const area::Entry> area::Area::FetchEntry(const area::Link link)
{
    if (link.page_no != current_page_idx_)
    {
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