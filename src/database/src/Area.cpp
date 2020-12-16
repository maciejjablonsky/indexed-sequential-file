#include "Area.hpp"

#include <filesystem>
#include <fmt/format.h>

void area::Area::AttachFile(std::string_view path, int page_size)
{
    page_size_ = page_size;
    file_ = std::make_unique<std::fstream>(path.data(), std::ios::binary);
    auto file_exists = std::filesystem::exists(path);
    if ((file_exists && std::filesystem::file_size(path) == 0) || !file_exists)
    {
        current_page_ = EntryPage(page_size_);
        current_page_idx_ = 0;
        pages_number_ = 1;
    }
    else
    {
        auto file_size = std::filesystem::file_size(path);
        pages_number_ = file_size / page_size_;
        current_page_idx_ = -1;
        current_page_ = std::nullopt;
    }
}

int area::Area::HowMuchPlaceLeft(area::Link link)
{
    if (link.page_no == previous_page_idx_)
    {
        return previous_page_->HowManyWillFit();
    }
    else if (link.page_no == current_page_idx_)
    {
        return current_page_->HowManyWillFit();
    }
    else
    {
        auto opt = ViewEntry(link);
    }
    return current_page_->HowManyWillFit();
}

void area::Area::View()
{
    area::Link link = {0, 0};
    while (link.page_no < pages_number_)
    {
        auto opt_entry = ViewEntry(link);
        if (!opt_entry)
        {
            return; 
        }
        const auto& entry = opt_entry->get();
        do
        {
            fmt::print(
                "[ {:>3} {:>3} | {:>6} | {:>15} | {:>3} {:>3} | {:<5} ]\n",
                link.page_no, link.entry_index,
                static_cast<std::string>(entry.key),
                static_cast<std::string>(entry.record), entry.link.page_no,
                entry.link.entry_index, entry.deleted);
            ++link.entry_index;
            opt_entry = ViewEntry(link);
        } while (opt_entry);
        link.entry_index = 0;
        ++link.page_no;
    }
}

optref<const area::Entry> area::Area::ViewEntry(area::Link link)
{
    if (link.page_no == current_page_idx_)
    {
        if (link.entry_index < current_page_->EntriesNumber())
        {
            return current_page_->AccessEntry(link.entry_index);
        }
        return std::nullopt;
    }
    else if (link.page_no == previous_page_idx_)
    {
        if (link.entry_index < current_page_->EntriesNumber())
        {
            return previous_page_->AccessEntry(link.entry_index);
        }
        return std::nullopt;
    }
    else
    {
        current_page_ = ReadDiskPage(link.page_no);
        current_page_idx_ = current_page_ ? link.page_no : -1;
    }
    if (current_page_)
    {
        if (link.entry_index < current_page_->EntriesNumber())
        {
            return current_page_->AccessEntry(link.entry_index);
        }
    }
    return std::nullopt;
}

std::pair<optref<const area::Entry>, area::Link> area::Area::ViewNextEntry(
    area::Link link)
{
    if (link.page_no >= pages_number_)
    {
        return {std::nullopt, {-1, -1}};
    }

    // look for index on the same page
    area::Link next_entry_link = {.page_no = link.page_no,
                                  .entry_index = link.entry_index + 1};
    auto opt_entry = ViewEntry(next_entry_link);
    if (opt_entry)
    {
        return {opt_entry, link};
    }
    


    // check next page
    area::Link next_page_entry_link = {.page_no = link.page_no + 1,
                                       .entry_index = 0};
    opt_entry = ViewEntry(next_page_entry_link);
    if (opt_entry)
    {
        return {opt_entry, next_page_entry_link};
    }
    return {std::nullopt, next_page_entry_link};
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

void area::Area::Insert(const area::Entry& entry, area::Link link)
{
    auto opt_new_entry = ViewEntry(link);
    const_cast<area::Entry&>(opt_new_entry->get()) = entry;
}

std::pair<optref<const area::Entry>, area::Link> area::Area::ViewLastEntry()
{
    area::Link link = {.page_no = pages_number_ - 1, .entry_index = 0};
    auto opt_entry = ViewEntry(link);
    if (!opt_entry)
    {
        return {std::nullopt, {-1, -1}};
    }
    link.entry_index = current_page_->EntriesNumber() - 1;
    return {ViewEntry(link), link};
}

std::pair<optref<area::Entry>, area::Link> area::Area::FetchLastEntry()
{
    area::Link link = {.page_no = pages_number_ - 1, .entry_index = 0};
    auto opt_entry = ViewEntry(link);
    if (!opt_entry)
    {
        return {std::nullopt, {-1, -1}};
    }
    link.entry_index = current_page_->EntriesNumber() - 1;
    return {FetchEntry(link), link};
}

std::pair<optref<const area::Entry>, area::Link>
area::Area::ViewLastEntryOnPage(area::Link link)
{
    auto opt_entry = ViewEntry(link);
    if (link.page_no == previous_page_idx_)
    {
        link.entry_index = previous_page_->EntriesNumber() - 1;
        return {ViewEntry(link), link};
    }
    else if (link.page_no == current_page_idx_)
    {
        link.entry_index = current_page_->EntriesNumber() - 1;
        return {ViewEntry(link), link};
    }
}

area::Link area::Area::InsertOnNewPage(const area::Entry& entry)
{
    if (previous_page_ && previous_page_->IsDirty())
    {
        WritePreviousDiskPage();
    }
    previous_page_ = std::move(current_page_);
    previous_page_idx_ = current_page_idx_;
    current_page_ = area::EntryPage(page_size_);
    ++pages_number_;
    ++current_page_idx_;
    current_page_->Write(entry);
    return {current_page_idx_, 0};
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

optref<area::Link> area::Area::Append(const area::Entry& entry)
{
    auto [opt_entry, link] = ViewLastEntry();
    if (HowMuchPlaceLeft(link) > 0)
    {
        ++link.entry_index;
        Insert(entry, link);
        return link;
    }
    return std::nullopt;
}

void area::Area::AppendToPage(const area::Entry& entry, area::Link link)
{
    auto opt = ViewEntry(link);
    if (link.page_no == previous_page_idx_)
    {
        previous_page_->Write(entry);
    }
    else if (link.page_no == current_page_idx_)
    {
        current_page_->Write(entry);
    }
}
