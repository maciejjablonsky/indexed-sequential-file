#ifndef COMMANDS_FILE_READER_HPP
#define COMMANDS_FILE_READER_HPP
#include <string>
#include <fstream>
#include <memory>

namespace commands
{
class FileReader
{
   public:
    FileReader(const std::string& path);
    [[nodiscard]] bool EndOfFile() const;
    [[nodiscard]] std::string ReadNextCommand();

   private:
    std::unique_ptr<std::ifstream> stream_;
    std::string path_;
};
}  // namespace commands

#endif  // COMMANDS_FILE_READER_HPP