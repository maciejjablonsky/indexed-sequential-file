#ifndef DATABASE_AREA_ENTRY_PAGE_HPP
#define DATABASE_AREA_ENTRY_PAGE_HPP

#include <vector>
#include "Entry.hpp"
namespace page
{

using PODPage = std::vector<std::byte>;

struct PODEntryPageHeader
{
    int32_t records_number;
};

template <typename Derived>
class Page
{
   public:
    using clean_entry =
        std::variant<const entries::CleanEntry, const entries::DeletedEntry>;
    Page(PODPage&& page);
    Page(size_t page_size);
    inline size_t Capacity() const
    {
        return (memory_.size() - sizeof(decltype(*header_))) /
               sizeof(entries::PODEntry);
    }
    inline size_t Size() const { return header_ ? header_->records_number : 0; }
    const clean_entry& View(size_t idx) const;
    PODPage&& Release();

   protected:
    inline size_t OffsetTo(size_t idx) const
    {
        return idx * sizeof(entries::PODEntry) + sizeof(PODEntryPageHeader);
    }
    PODPage memory_;
    PODEntryPageHeader* header_;
};

class CleanPage : public Page<CleanPage>
{
};

class DirtyPage : public Page<DirtyPage>
{

   public:
    DirtyPage(CleanPage&& clean_page);
    DirtyPage& operator=(CleanPage&& clean_page);
    void Write(const entries::PODEntry& entry, size_t idx);
    void Append(const entries::PODEntry& entry);
};

template <typename Derived>
inline Page<Derived>::Page(PODPage&& page)
    : memory_(std::move(page)),
      header_(reinterpret_cast<PODEntryPageHeader*>(memory_.data()))
{
}
template <typename Derived>
inline Page<Derived>::Page(size_t page_size)
    : memory_(page_size),
      header_(reinterpret_cast<PODEntryPageHeader*>(memory_.data()))
{
}
template <typename Derived>
inline const Page<Derived>::clean_entry& Page<Derived>::View(size_t idx) const
{
    if (idx >= Count())
    {
        throw std::out_of_range("Attempted to view entry outside range.");
    }
    auto offset = sizeof(*header_) + Count() * idx;
    return *reinterpret_cast<const Entry*>(memory_.data() + offset);
}
template <typename Derived>
inline PODPage&& Page<Derived>::Release()
{
    header_ = nullptr;
    return std::move(memory_);
}
}  // namespace page

#endif  // DATABASE_AREA_ENTRY_PAGE_HPP