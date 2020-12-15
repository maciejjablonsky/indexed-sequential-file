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
namespace area
{
class Area
{
   public:
    [[nodiscard]] optref<const area::Entry> ViewEntry(area::Link link);
    [[nodiscard]] optref<const area::Entry> ViewNextEntry(area::Link link);
    [[nodiscard]] optref<area::Entry> FetchEntry(area::Link link);
    [[nodiscard]] optref<area::Entry> FetchNextEntry(area::Link link);
    void AppendEntry(area::Entry&& entry);
    void AttachFile(std::string_view path, int page_size);
    [[nodiscard]] area::Link CreatePageWithEntry(area::Entry&& entry);
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