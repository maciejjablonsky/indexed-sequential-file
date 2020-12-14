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
    [[nodiscard]] optref<const area::Entry> FetchEntry(const area::Link link);
    [[nodiscard]] optref<const area::Entry> FetchNextEntry(const area::Link link);
    void AttachFile(std::string_view path, int page_size);
    [[nodiscard]] area::Link CreatePageWithEntry(area::Entry&& entry);
   private:
    std::optional<std::vector<std::byte>> ReadDiskPage(int idx);
    void WriteDiskPage();
    std::unique_ptr<std::fstream> file_;
    int pages_number_ = 0;
    int current_page_idx_ = -1;
    int page_size_ = 4096;
    std::optional<area::EntryPage> current_page_;
};
}  // namespace area
#endif  // DATABASE_AREA_HPP