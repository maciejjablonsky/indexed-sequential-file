#pragma once

#include "DiskAccess.hpp"
#include <array>
#include <concepts/memory_access.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <wrappers/opt.hpp>

namespace page {

template <typename Memory> requires memory_access<Memory> class PageDispositor {
  private:
    std::string file_path_;
    size_t pages_in_file_;
    std::unique_ptr<std::fstream> file_;
    DiskAccess counter_ = {0, 0};

  private:
    Memory Make() {
        auto tmp = Memory(pages_in_file_);
        ++pages_in_file_;
        return tmp;
    }

  public:
    bool AttachFile(const std::string &file_path) {
        file_path_ = file_path;
        if (std::filesystem::exists(file_path)) {
            file_ = std::make_unique<std::fstream>(
                file_path, std::ios::binary | std::ios::in | std::ios::out);
            pages_in_file_ =
                std::filesystem::file_size(file_path) / Memory::size;
        } else {
            file_ = std::make_unique<std::fstream>(
                file_path, std::ios::binary | std::ios::in | std::ios::out |
                               std::ios::trunc);
            pages_in_file_ = 0;
        }
        if (!file_ || !file_->is_open()) {
            throw std::runtime_error(
                fmt::format("Failed to open file: {}", file_path_));
        }
        return true;
    };
    std::optional<Memory> Request(size_t page_index) {
        if (!file_ || !file_->is_open()) {
            throw std::runtime_error("No file set to page dispositor.");
        }
        if (page_index > pages_in_file_) {
            return std::nullopt;
        } else if (page_index == pages_in_file_) {
            return Make();
        } else {
            file_->seekg(page_index * Memory::size);
            continuous_memory tmp(Memory::size);
            file_->read(reinterpret_cast<char *>(tmp.data()), Memory::size);
            counter_.Read();
            Memory mem(page_index, std::move(tmp));
            return std::move(mem);
        }
    }
    void Write(const Memory &page) {
        if (!file_ || !file_->is_open()) {
            throw std::runtime_error("No file set to page dispositor.");
        }
        file_->seekg(page.Index() * Memory::size);
        file_->write(reinterpret_cast<const char *>(page.data()), Memory::size);
        counter_.Write();
    }
    inline size_t PagesInFile() const { return pages_in_file_; }
    [[nodiscard]] inline auto CountReads() const {
        return counter_.CountReads();
    }
    [[nodiscard]] inline auto CountWrites() const {
        return counter_.CountWrites();
    }
    void ClearFile() {
        if (std::filesystem::exists(file_path_)) {
            file_ = std::make_unique<std::fstream>(
                file_path_, std::ios::binary | std::ios::in | std::ios::out |
                                std::ios::trunc);
            if (!file_ || !file_->is_open()) {
                throw std::runtime_error(
                    fmt::format("Failed to open file at: {}\n", file_path_));
            }
            pages_in_file_ = 0;
        }
    }
    inline DiskAccess GetDiskAccessCounter() const { return counter_; }
    inline void SetDiskAccessCounter(const DiskAccess &new_counter) {
        counter_ = new_counter;
    }

    inline void CloseFile() {
        if (file_) {
            file_.reset();
        }
    }
};

} // namespace page