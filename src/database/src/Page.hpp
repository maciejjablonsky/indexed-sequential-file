#ifndef DATABASE_AREA_ENTRY_PAGE_HPP
#define DATABASE_AREA_ENTRY_PAGE_HPP

#include <vector>
#include <stdexcept>

namespace page
{
template <typename PageMemory>
concept memory_access = requires
{
    PageMemory::data(), PageMemory::size(), PageMemory::begin(), PageMemory::end();
};
template <typename PageMemory, typename Entry>
requires memory_access<PageMemory>&& std::is_trivial_v<Entry> class PageWithEntries
{
   public:
    PageWithEntries(PageMemory&& page);
    inline size_t Capacity() const { return (memory_.size() - sizeof(*header_)) / sizeof(Entry); }
    inline size_t Size() const { return header_ ? header_->records_number : 0; }
    const Entry& View(size_t idx) const;
    PageMemory&& Release();

   protected:
    size_t OffsetTo(size_t idx) const;
    PageMemory memory_;
    struct
    {
        size_t entries;
    } * header_;
};

template <typename PageMemory, typename Entry>
class CleanPageWithEntries : public PageWithEntries<PageMemory, Entry>
{
};

template <typename PageMemory, typename Entry>
class DirtyPageWithEntries : public Page<PageMemory, Entry>
{
   public:
    DirtyPageWithEntries(CleanPageWithEntries<PageMemory, Entry>&& clean_page);
    DirtyPageWithEntries& operator=(CleanPageWithEntries<PageMemory, Entry>&& clean_page);
    void Write(const Entry& entry, size_t idx);
    void Append(const Entry& entry);
};
template <typename PageMemory, typename Entry>
inline PageWithEntries<PageMemory, Entry>::PageWithEntries(PageMemory&& page)
    : memory_(std::move(page)), header_(reinterpret_cast<Header*>(memory_.data()))
{
}
template <typename PageMemory, typename Entry>
inline PageMemory&& PageWithEntries<PageMemory, Entry>::Release()
{
    header_ = nullptr;
    return std::move(memory_);
}
template <typename PageMemory, typename Entry>
inline size_t PageWithEntries<PageMemory, Entry>::OffsetTo(size_t idx) const
{
    return sizeof(Header) + idx * sizeof(Entry);
}

template <typename PageMemory, typename Entry>
inline const Entry& PageWithEntries<PageMemory, Entry>::View(size_t idx) const 
{
    if (idx >= Size())
    {
        throw std::out_of_range("Attempted to read behind page index.");
    }
    return *reinterpret_cast<const Entry&>(memory_.data() + OffsetTo(idx));
}

template <typename PageMemory, typename Entry>
inline DirtyPageWithEntries<PageMemory, Entry>::DirtyPageWithEntries(
    CleanPageWithEntries<PageMemory, Entry>&& clean_page)
    : PageWithEntries<PageMemory, Entry>(clean_page.Release())
{
}

template <typename PageMemory, typename Entry>
inline DirtyPageWithEntries<PageMemory, Entry>& DirtyPageWithEntries<PageMemory, Entry>::operator=(
    CleanPageWithEntries<PageMemory, Entry>&& clean_page)
{
    memory_ = clean_page.Release();
    header_ = reinterpret_cast<Header*>(memory_.data());
}

template <typename PageMemory, typename Entry>
void page::DirtyPageWithEntries<PageMemory, Entry>::Write(const Entry& entry, size_t idx)
{
    if (idx > Size())
    {
        throw std::out_of_range("Attempted to write behind page index.");
    }
    if (idx == Capacity())
    {
        throw std::out_of_range("Page is full.");
    }
    ++header_->entries;
    std::copy_n(
        &entry, 1,
        reinterpret_cast<Entry*>(memory_.data() + OffsetTo(idx)));
}

template <typename PageMemory, typename Entry>
void page::DirtyPageWithEntries<PageMemory, Entry>::Append(const Entry & entry)
{
    Write(entry, Size());
}
}  // namespace page

#endif  // DATABASE_AREA_ENTRY_PAGE_HPP