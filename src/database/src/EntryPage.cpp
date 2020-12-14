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

int area::EntryPage::HowManyWillFit() const {
    return max_records_number_ - header_->records_number;
}

const area::Entry & area::EntryPage::Read(int idx)
{
    if (idx >= max_records_number_)
    {
        throw std::out_of_range("Record index is too far.");
    }
    return GetEntry(idx);
}

void area::EntryPage::Write(area::Entry&& new_entry, int idx)
{
    dirty_ = true;
    if (idx >= max_records_number_ || idx > header_->records_number)
    {
        throw std::out_of_range("Cannot write record so far.");
    }
    auto& entry_place = GetEntry(idx);
    entry_place = std::move(new_entry);
    header_->records_number += (idx == header_->records_number);
}

bool area::EntryPage::IsDirty() const { return dirty_; }

void area::EntryPage::SetDirty() { dirty_ = true; }

void area::EntryPage::ClearDirty() { dirty_ = false; }

std::byte* area::EntryPage::Data() { return memory_.data(); }

void area::EntryPage::Write(area::Entry&& new_entry)
{
    Write(std::move(new_entry), header_->records_number);
}

area::Entry & area::EntryPage::GetEntry(int idx)
{
    auto offset = sizeof(*header_) + idx * sizeof(area::Entry);
    return *reinterpret_cast<area::Entry*>(memory_.data() + offset);
}