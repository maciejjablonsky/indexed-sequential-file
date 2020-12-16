#ifndef DATABASE_AREA_ENTRY_PAGE_HPP
#define DATABASE_AREA_ENTRY_PAGE_HPP

#include <vector>
namespace area
{
struct EntryPageHeader
{
    int32_t records_number;
};

template <typename T>
class EntryPage
{
   public:
    EntryPage() = default;
    EntryPage(int page_size);
    EntryPage(std::vector<std::byte>&& memory);
    [[nodiscard]] inline int CountPlacesLeft() const
    {
        return entries_max_ - header_->records_number;
    }
    [[nodiscard]] inline int Count() const { return header_->records_number; }
    [[nodiscard]] inline int Max() const { return enties_max_; }
    [[nodiscard]] T& AccessEntry(int idx);
    void Append(const T& entry);
    void Write(const T& entry, int idx);
    [[nodiscard]] inline bool IsDirty() const { return dirty_; }
    inline void SetDirty() { dirty_ = true; }
    inline void ClearDirty() { dirty_ = false; }
    inline std::byte* Data() { return memory_.data(); }

   private:
    int page_size_ = 0;
    int entries_max_ = 0;
    bool dirty_ = false;
    std::vector<std::byte> memory_;
    EntryPageHeader* header_ = nullptr;
};

template <typename T>
inline EntryPage<T>::EntryPage(int page_size)
    : page_size_(page_size),
      entries_max_((page_size_ - sizeof(EntryPageHeader)) / sizeof(T)),
      memory_(page_size),
      header_(reinterpret_cast<T*>(memory_.data()))
{
}
template <typename T>
inline EntryPage<T>::EntryPage(std::vector<std::byte>&& memory)
    : page_size_(memory.size()),
      entries_max_((page_size_ - sizeof(EntryPageHeader)) / sizeof(T)),
      memory_(std::move(memory)),
      header_(reinterpret_cast<T*>(memory_.data()))
{
}
template <typename T>
inline T& EntryPage<T>::AccessEntry(int idx)
{
    if (idx >= Count())
    {
        throw std::out_of_range("Attempted to read behind page entry index.");
    }
    auto offset = sizeof(*header_) + idx * sizeof(T);
    return *reinterpret_cast<T*>(memory_.data() + offset);
}
template <typename T>
inline void EntryPage<T>::Append(const T& entry)
{
    Write(entry, header_->records_number);
}
template <typename T>
inline void EntryPage<T>::Write(const T& entry, int idx)
{
    if (idx >= std::min(Count(), Max()))
    {
        throw std::out_of_range("Attempted to write behind page entry index.");
    }
    SetDirty();
    auto& entry_place = AccessEntry(idx);
    entry_place = entry;
    header_->records_number += (idx == header_->records_number);
}
}  // namespace area

#endif  // DATABASE_AREA_ENTRY_PAGE_HPP