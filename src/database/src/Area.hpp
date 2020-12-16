#ifndef AREA_HPP
#define AREA_HPP
#include "EntryPage.hpp"
#include "Link.hpp"
#include <optional>
#include <utility>
#include <fstream>
#include "DiskAccess.hpp"
#include <type_traits>

template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;
template <typename T>
using ref = std::reference_wrapper<T>;

namespace area
{
template <typename T>
struct StoredPage
{
    EntryPage<T> page;
    int index;
};
template <typename T>
class Area
{
   public:
    Area() = default;
    Area(int page_size);

    void Attach(const std::string& path);
    [[nodiscard]] bool Insert(const T& entry, area::Link link);

    const T& ViewEntry(area::Link link) const;
    T& FetchEntry(area::Link link);
    std::pair<const T&, area::Link> ViewSubsequentEntry(area::Link link) const;
    std::pair<T&, area::Link> FetchSubsequentEntry(area::Link link);

   private:
    [[nodiscard]] const EntryPage<T>& ViewEntryPage(int page_no) const;
    [[nodiscard]] EntryPage<T>& FetchEntryPage(int page_no);
    void AddDirtyEntryPage();
    void BufferLoadedPage() const;
    [[nodiscard]] bool ReadEntryPage(int page_no) const;

   private:
    mutable std::unique_ptr<std::fstream> file_;
    int page_size_ = 4096;
    mutable std::optional<StoredPage<T>> loaded_;
    mutable std::optional<StoredPage<T>> buffered_;
    int pages_number_ = 0;
    mutable DiskAccess counter_;
};
template <typename T>
inline area::Area<T>::Area(int page_size) : page_size_(page_size), counter_({})
{
}
template <typename T>
inline void Area<T>::Attach(const std::string& path)
{
    file_ = std::make_unique<std::fstream>(path.c_str());
}
template <typename T>
inline bool Area<T>::Insert(const T& entry, area::Link link)
{
    auto& page = FetchEntryPage(link.page_no);
    page.Write(entry, link.entry_index);
}
template <typename T>
inline const T& Area<T>::ViewEntry(area::Link link) const
{
    const auto& page = ViewEntryPage(link.page_no);
    return page.View(link.entry_index);
}
template <typename T>
inline T& Area<T>::FetchEntry(area::Link link)
{
    return const_cast<T&>(const_cast<const Area<T>&>(*this).ViewEntry(link));
}
template <typename T>
inline std::pair<const T&, area::Link> Area<T>::ViewSubsequentEntry(
    area::Link link) const
{
    const auto&
}
template <typename T>
inline const EntryPage<T>& Area<T>::ViewEntryPage(int page_no) const
{
    if (buffered_ && buffered_->index == page_no)
    {
        return buffered_->page;
    }
    if (loaded_ && loaded_->index == page_no)
    {
        return loaded_->page;
    }
    else
    {
        if (!ReadEntryPage(page_no))
        {
            throw std::runtime_error(fmt::format(
                "Attempted to read not existing page no {:02}", page_no));
        }
    }
    return loaded_->page;
}
template <typename T>
inline EntryPage<T>& Area<T>::FetchEntryPage(int page_no)
{
    auto& mutable_page = const_cast<area::EntryPage<T>&>(
        const_cast<const Area<T>&>(*this).ViewEntryPage(page_no));
    mutable_page.SetDirty();
    return mutable_page;
}
template <typename T>
inline void Area<T>::AddDirtyEntryPage()
{
    BufferLoadedPage();
    loaded_ =
        StoredPage<T>{.page = EntryPage<T>(page_size_), .index = pages_number_};
    loaded_->page.SetDirty();
    ++pages_number_;
}
template <typename T>
inline void Area<T>::BufferLoadedPage() const
{
    if (buffered_ && buffered_->page.IsDirty())
    {
        if (!file_)
        {
            throw std::runtime_error("No file attached to area.");
        }
        file_->seekg(
            static_cast<std::streampos>(buffered_->index * page_size_));
        file_->write(reinterpret_cast<char*>(buffered_->page.Data()),
                     page_size_);
        buffered_->page.ClearDirty();
        counter_.Write();
    }
    buffered_ = std::move(loaded_);
}
template <typename T>
inline bool Area<T>::ReadEntryPage(int page_no) const
{
    if (!file_)
    {
        throw std::runtime_error("No file attached to area.");
    }
    if (page_no >= pages_number_)
    {
        throw std::out_of_range("Attempted to read page too far.");
    }
    BufferLoadedPage();
    std::vector<std::byte> tmp_memory(page_size_);
    file_->read(reinterpret_cast<char*>(tmp_memory.data()), page_size_);
    if (file_->eof())
    {
        return false;
    }
    loaded_->page = area::EntryPage<T>(std::move(tmp_memory));
    loaded_->index = page_no;
    counter_.Read();
    return true;
}
}  // namespace area
#endif  // AREA_HPP