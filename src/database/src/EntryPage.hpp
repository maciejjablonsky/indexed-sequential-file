#ifndef DATABASE_AREA_ENTRY_PAGE_HPP
#define DATABASE_AREA_ENTRY_PAGE_HPP

#include "Entry.hpp"
#include <vector>
namespace area
{
struct EntryPageHeader
{
    int32_t records_number;
};
class EntryPage
{
   public:
    EntryPage() = default;
    EntryPage(int page_size);
    EntryPage(std::vector<std::byte>&& memory);
    [[nodiscard]] inline int HowManyWillFit() const;
    [[nodiscard]] const area::Entry& Read(int idx);
    void Write(area::Entry&& entry);
    void Write(area::Entry&& entry, int idx);
    void SetMemory(std::vector<std::byte>&& memory);
    [[nodiscard]] inline bool IsDirty() const;
    void SetDirty();
    void ClearDirty();
    std::byte* Data();

   private:
    area::Entry& GetEntry(int idx);
    int page_size_ = 0;
    int max_records_number_ = 0;
    bool dirty_ = false;
    std::vector<std::byte> memory_;
    EntryPageHeader* header_ = nullptr;
};
}  // namespace area

#endif  // DATABASE_AREA_ENTRY_PAGE_HPP