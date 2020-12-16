#ifndef DATABASE_AREA_HPP
#define DATABASE_AREA_HPP

#include <optional>
#include <utility>
#include <database/Record.hpp>
#include "Link.hpp"
#include <database/Key.hpp>
#include "Entry.hpp"
#include "EntryPage.hpp"
#include <fstream>
template <typename T>
using optref = std::optional<std::reference_wrapper<T>>;
template <typename T>
using ref = std::reference_wrapper<T>;
namespace area
{
class Area
{
   public:
    [[nodiscard]] optref<const area::Entry> ViewEntry(area::Link link);
    [[nodiscard]] std::pair<optref<const area::Entry>, area::Link> ViewNextEntry(area::Link link);
    [[nodiscard]] optref<area::Entry> FetchEntry(area::Link link);
    [[nodiscard]] optref<area::Entry> FetchNextEntry(area::Link link);
    void Insert(const area::Entry& entry, area::Link link);
    optref<area::Link> Append(const area::Entry& entry);
    void AppendToPage(const area::Entry& entry, area::Link link);
    [[nodiscard]] std::pair<optref<const area::Entry>, area::Link>
    ViewLastEntry();
    [[nodiscard]] std::pair<optref<area::Entry>, area::Link> FetchLastEntry();
    [[nodiscard]] std::pair<optref<const area::Entry>, area::Link>
    ViewLastEntryOnPage(area::Link link);
    [[nodiscard]] std::pair<optref<area::Entry>, area::Link>
    FetchLastEntryOnPage(area::Link link);
    [[nodiscard]] area::Link InsertOnNewPage(const area::Entry& entry);
    void AttachFile(std::string_view path, int page_size);
    [[nodiscard]] int HowMuchPlaceLeft(area::Link link);
    void View();
    inline int MaxEntriesNumber() const {
        return  
    }

   private:
    std::optional<area::EntryPage> ReadDiskPage(int idx);
    void WritePreviousDiskPage();
    void WriteDiskPage();
    std::unique_ptr<std::fstream> file_;
    int pages_number_ = 0;
    int current_page_idx_ = -1;
    int previous_page_idx_ = -1;
    int page_size_ = 4096;
    std::optional<area::EntryPage> previous_page_;
    std::optional<area::EntryPage> current_page_;
};
}  // namespace area
#endif  // DATABASE_AREA_HPP