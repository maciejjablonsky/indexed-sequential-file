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

optref<const area::Entry> area::Area::ViewEntry(area::Link link)
{
    if (link.page_no == current_page_idx_)
    {
        return current_page_->AccessEntry(link.entry_index);
    }
    else if (link.page_no == previous_page_idx_)
    {
        return previous_page_->AccessEntry(link.entry_index);
    }
    else
    {
        current_page_ = ReadDiskPage(link.page_no);
        current_page_idx_ = current_page_ ? link.page_no : -1;
    }
    if (current_page_)
    {
        return current_page_->AccessEntry(link.entry_index);
    }
    return std::nullopt;
}

optref<const area::Entry> area::Area::ViewNextEntry(area::Link link)
{
    ++link.entry_index;
    return ViewEntry(link);
}

optref<area::Entry> area::Area::FetchEntry(area::Link link)
{
    auto entry = ViewEntry(link);
    if (entry)
    {
        if (entry->get().link.page_no == current_page_idx_)
        {
            current_page_->SetDirty();
        }
        else
        {
            previous_page_->SetDirty();
        }
    }
    return const_cast<area::Entry&>(entry->get());
}

optref<area::Entry> area::Area::FetchNextEntry(area::Link link)
{
    link.entry_index += 1;
    return FetchEntry(link);
}

void area::Area::AppendEntry(area::Entry&& entry)
{
    // TODO
}

std::optional<area::EntryPage> area::Area::ReadDiskPage(int idx)
{
    if (previous_page_ && previous_page_->IsDirty())
    {
        WritePreviousDiskPage();
    }
    previous_page_ = std::move(current_page_);
    previous_page_idx_ = current_page_idx_;
    std::vector<std::byte> tmp_memory(page_size_);
    file_->read(reinterpret_cast<char*>(tmp_memory.data()), page_size_);
    if (file_->eof())
    {
        return std::nullopt;
    }
    area::EntryPage tmp_page(std::move(tmp_memory));
    return std::move(tmp_page);
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
    file_->seekg(static_cast<std::streampos>(current_page_idx_ * page_size_));
    file_->write(reinterpret_cast<char*>(current_page_->Data()), page_size_);
    current_page_->ClearDirty();
}

void area::Area::WritePreviousDiskPage()
{
    if (!file_)
    {
        throw std::runtime_error("No file set to write.");
    }
    if (!previous_page_)
    {
        throw std::runtime_error("No page set to write");
    }
    file_->seekg(static_cast<std::streampos>(previous_page_idx_ * page_size_));
    file_->write(reinterpret_cast<char*>(previous_page_->Data()), page_size_);
    previous_page_->ClearDirty();
}
