#pragma once

#include <concepts/memory_access.hpp>
#include <stdexcept>
#include <vector>
#include <wrappers/optref.hpp>

namespace page {
template <typename PageMemory, typename Entry>
concept page_with_entries = requires(PageMemory memory) {
    memory_access<PageMemory>, memory.Index(), std::is_trivial_v<Entry>;
};

template <typename PageMemory, typename Entry>
requires page_with_entries<PageMemory, Entry> class PageWithEntries {
  protected:
    PageMemory memory_;
    struct Header {
        size_t entries;
    };
    Header *header_ = nullptr;

  protected:
    size_t OffsetTo(size_t idx) const {
        return sizeof(Header) + idx * sizeof(Entry);
    }

  public:
    inline PageWithEntries(PageMemory &&page)
        : memory_(std::move(page)),
          header_(reinterpret_cast<Header *>(memory_.data())) {}

    inline size_t Capacity() const {
        return (PageMemory::size - sizeof(*header_)) / sizeof(Entry);
    }
    inline size_t Size() const { return header_ ? header_->entries : 0; }
    inline size_t Index() const { return memory_.Index(); }

    wr::optional_ref<const Entry> View(size_t idx) const {
        if (idx >= Size()) {
            return std::nullopt;
        }
        return *reinterpret_cast<const Entry *>(memory_.data() + OffsetTo(idx));
    }
    PageMemory &&Release() {
        header_ = nullptr;
        return std::move(memory_);
    }
    const PageMemory &Memory() const { return memory_; }
    PageMemory &Memory() {
        using ThisT = PageWithEntries<PageMemory, Entry>;
        return const_cast<PageMemory &>(
            const_cast<const ThisT &>(*this).Memory());
    }
};

template <typename PageMemory, typename Entry>
class CleanPageWithEntries : public PageWithEntries<PageMemory, Entry> {
  public:
    inline CleanPageWithEntries(PageMemory &&page)
        : PageWithEntries<PageMemory, Entry>(std::move(page)) {}
};

template <typename PageMemory, typename Entry>
class DirtyPageWithEntries : public PageWithEntries<PageMemory, Entry> {
  public:
    DirtyPageWithEntries(CleanPageWithEntries<PageMemory, Entry> &&clean_page)
        : PageWithEntries<PageMemory, Entry>(clean_page.Release()) {}
    void Write(const Entry &entry, size_t idx) {
        if (idx > Size()) {
            throw std::out_of_range("Attempted to write behind page index.");
        }
        if (idx == Capacity()) {
            throw std::out_of_range("Page is full.");
        }
        if (idx == Size()) {
        ++header_->entries;
        }
        std::copy_n(&entry, 1,
                    reinterpret_cast<Entry *>(memory_.data() + OffsetTo(idx)));
    }
    size_t Append(const Entry &entry) {
        Write(entry, Size());
        return Size() - 1;
    }
};
} // namespace page
