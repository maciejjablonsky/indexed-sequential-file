#include "EntryPage.hpp"

area::EntryPage::EntryPage(int page_size)
    : page_size_(page_size),
      max_records_number_(
          (page_size_ - sizeof(area::EntryPageHeader)) / sizeof(area::Entry)),
      memory_(page_size),
      header_(reinterpret_cast<area::EntryPageHeader*>(memory_.data()))
{
}

area::EntryPage::EntryPage(std::vector<std::byte>&& memory)
    : page_size_(memory.size()),
      max_records_number_(
          (page_size_ - sizeof(area::EntryPageHeader)) / sizeof(area::Entry)),
      memory_(std::move(memory)),
      header_(reinterpret_cast<area::EntryPageHeader*>(memory_.data()))
{
}

area::Entry & area::EntryPage::AccessEntry(int idx)
{
    if (idx >= max_records_number_)
    {
        throw std::out_of_range("Record index is too far.");
    }
    auto offset = sizeof(*header_) + idx * sizeof(area::Entry);
    return *reinterpret_cast<area::Entry*>(memory_.data() + offset);
}

void area::EntryPage::Write(const area::Entry& new_entry, int idx)
{
    dirty_ = true;
    if (idx >= max_records_number_ || idx > header_->records_number)
    {
        throw std::out_of_range("Cannot write record so far.");
    }
    auto& entry_place = AccessEntry(idx);
    entry_place = new_entry;
    header_->records_number += (idx == header_->records_number);
}

void area::EntryPage::Write(const area::Entry& new_entry)
{
    Write(new_entry, header_->records_number);
}
