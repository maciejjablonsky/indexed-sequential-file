#include "Page.hpp"

page::DirtyPage::DirtyPage(CleanPage&& clean_page)
    : page::Page<DirtyPage>(clean_page.Release())
{
}

page::DirtyPage& page::DirtyPage::operator=(CleanPage&& clean_page)
{
    memory_ = clean_page.Release();
    header_ = reinterpret_cast<page::PODEntryPageHeader*>(memory_.data());
}

void page::DirtyPage::Write(const entries::PODEntry& entry, size_t idx)
{
    if (idx > Size())
    {
        throw std::out_of_range("Attempted to write behind page index.");
    }
    if (idx == Capacity())
    {
        throw std::out_of_range("Page is full.");
    }
    std::copy_n(
        &entry, 1,
        reinterpret_cast<entries::PODEntry*>(memory_.data() + OffsetTo(idx)));
}

void page::DirtyPage::Append(const entries::PODEntry& entry)
{
    Write(entry, Size());
}
